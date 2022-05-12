#include "StudyLoggerSingleton.h"
#include "Engine/World.h"
#include "IDisplayCluster.h"
#include "HAL/FileManagerGeneric.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Engine.h"

USFLoggingSingleton::USFLoggingSingleton(){}

//////////////////////////---Initialize---//////////////////////////
void USFLoggingSingleton::Initialize(uint32 ProbandIDval) {
	if (FModuleManager::Get().IsModuleLoaded("nDisplay")
		&& IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster) { // Cave
		MasterNode = IDisplayCluster::Get().GetClusterMgr()->GetNodeId().Equals(TEXT("node_main"));
	}
	else { // Desktop oder HMD
		MasterNode = true;
	}
	StaticDateTime = FDateTime::Now();
	ProbandID = ProbandIDval;
	LoggingInfo.Empty();
	SetUpLogDir();
}

void USFLoggingSingleton::Initialize() {
	Initialize(-1);
}

void USFLoggingSingleton::Destruct()
{
	CustomEventFileHandle.Reset();
	CondensedFileHandle.Reset();
	ActorEventFileHandle.Reset();

	LoggingInfo.Empty();
}

void USFLoggingSingleton::Initialize(FString FilePath) {
	SetUpProbandID(FilePath);
	Initialize(ProbandID);
}

//////////////////////////---AddActor---//////////////////////////
void USFLoggingSingleton::AddActor(AActor* Actor, int32 LogTimer) {
	AddActorWithName(Actor, LogTimer, Actor->GetFName().ToString());
}

void USFLoggingSingleton::AddActorWithName(AActor* Actor, int32 LogTimer, FString LogName) {
	LoggingInfo.Emplace(LogTimer, Actor, LogName, IndividualActorsDir);
}

void USFLoggingSingleton::AddActors(TArray<AActor*> ActorArray, int32 LogTimer) {
	for(AActor* a : ActorArray){
		AddActor(a, LogTimer);
	}
}

//////////////////////////---SetUps---//////////////////////////
void USFLoggingSingleton::SetUpLogDir() {
	if (ProbandID < 0) {
		LogDir = FString::Printf(TEXT("%s/StudyLogs/%s"), *FPaths::ProjectDir(), *FDateTime::Now().ToString());
	} else {
		LogDir = FString::Printf(TEXT("%s/StudyLogs/Proband_%i_%s"), *FPaths::ProjectDir(), ProbandID, *FDateTime::Now().ToString());
	}
	
	CondensedActorFile = LogDir + "/CondensedActors/ActorLogs.txt";
	IndividualActorsDir = LogDir + "/IndividualActors";
	CustomEventsFile = LogDir + "/EventLogs.txt";
	ActorEventsFile = LogDir + "/ActorEventLogs.txt";
}

void USFLoggingSingleton::SetUpProbandID(FString FilePath) {
	if (!GConfig->GetInt(TEXT("Proband"), TEXT("ID"), ProbandID, FilePath)) {
		ProbandID = -1;
	}
}


//////////////////////////---WriteToFile---//////////////////////////
void USFLoggingSingleton::WriteToFile(TUniquePtr<IFileHandle> & IndividualFileHandle, FString & String, FString & ActorFile, FString & Header){
	if (!IndividualFileHandle) {
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*FPaths::GetPath(ActorFile));
		IndividualFileHandle.Reset(PlatformFile.OpenWrite(*ActorFile, false, true));

		FTCHARToUTF8 UTF8String(*Header);
		IndividualFileHandle->Write((UTF8CHAR*)UTF8String.Get(), UTF8String.Length() * sizeof(UTF8CHAR));
	}
	if (IndividualFileHandle) {
		FTCHARToUTF8 UTF8String(*String);
		IndividualFileHandle->Write((UTF8CHAR*)UTF8String.Get(), UTF8String.Length() * sizeof(UTF8CHAR));
	}
}

void USFLoggingSingleton::WriteToCondensedFile(FString &String) {
	
	FString FileHeader = "ProbandID\tTimeSinceStart\t";
	if(!CondensedFileHandle){ //not needed anymore if file is initialized, thus not computed
		for (FLoggingInfo & ActorInfo : LoggingInfo) {
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_X_Pos\t";
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_Y_Pos\t";
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_Z_Pos\t";
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_Pitch_Ori\t";
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_Yaw_Ori\t";
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_Roll_Ori\t";
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_X_Quat\t";
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_Y_Quat\t";
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_Z_Quat\t";
			FileHeader += ActorInfo.ActorToLog->GetFName().ToString() + "_W_Quat\t";
		}
		FileHeader = FileHeader + "\n";
	}

	WriteToFile(CondensedFileHandle, String, CondensedActorFile, FileHeader);
}

void USFLoggingSingleton::WriteToEventLogsFile(FString& String)
{
	WriteToFile(CustomEventFileHandle, String, CustomEventsFile, EventLogsFileHeader);
}

void USFLoggingSingleton::WriteToActorEventFile(FString& String)
{
	WriteToFile(ActorEventFileHandle, String, ActorEventsFile, ActorEventFileHeader);
}

void USFLoggingSingleton::WriteToIndividualFile(TUniquePtr<IFileHandle> & IndividualFileHandle, FString& String, FString& ActorFile)
{
	WriteToFile(IndividualFileHandle, String, ActorFile, IndividualFileHeader);
}
