// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SFLoggingBPLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"
#include "HAL/FileManagerGeneric.h"
#include "LoggingPlugin.h"
#include "Engine/Engine.h"
#include "StudyLoggerSingleton.h"

USFLoggingBPLibrary::USFLoggingBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
}
/////--Initialize--/////
void USFLoggingBPLibrary::Initialize() {
	GetInstance()->Initialize();
}

void USFLoggingBPLibrary::Destruct()
{
	GetInstance()->Destruct();
}

void USFLoggingBPLibrary::InitializeWithConfig(FString FilePath) {
	GetInstance()->Initialize(FilePath);
}

/////--AddActors--/////
void USFLoggingBPLibrary::AddActorToLog(AActor* Actor, int32 LogTimer) {
	if (!GetInstance()->MasterNode) return;
	GetInstance()->AddActor(Actor, LogTimer);
}

void USFLoggingBPLibrary::AddActorToLogWithName(AActor* Actor, int32 LogTimer, FString LogName) {
	if (!GetInstance()->MasterNode) return;
	GetInstance()->AddActorWithName(Actor, LogTimer, LogName);
}

void USFLoggingBPLibrary::AddActorsToLog(TArray<AActor*> ActorArray, int32 LogTimer) {
	if (!GetInstance()->MasterNode) return;
	GetInstance()->AddActors(ActorArray, LogTimer);
}


/////--LogActors--/////
void USFLoggingBPLibrary::LogToCondensedFile(int32 Timer) {
	
	if (!GetInstance()->MasterNode) return;
	
	if ((FDateTime::Now() - GetInstance()->StaticDateTime).GetTotalMilliseconds() > Timer || Timer <= 0) {
		GetInstance()->StaticDateTime = FDateTime::Now();
		
		FString Row = "";

		if(GetInstance()->LoggingInfo.Num() > 0){
			Row += FString::Printf(TEXT("%i\t%f\t"), GetInstance()->ProbandID,	GetInstance()->LoggingInfo[0].ActorToLog->GetWorld()->GetTimeSeconds());
		}
		
		for (auto& ActorLoggingInfo : GetInstance()->LoggingInfo) {
			Row +=
				FString::Printf(TEXT("%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t"),
						ActorLoggingInfo.ActorToLog->GetActorLocation().X,
						ActorLoggingInfo.ActorToLog->GetActorLocation().Y,
						ActorLoggingInfo.ActorToLog->GetActorLocation().Z,
						ActorLoggingInfo.ActorToLog->GetActorRotation().Pitch,
						ActorLoggingInfo.ActorToLog->GetActorRotation().Yaw,
						ActorLoggingInfo.ActorToLog->GetActorRotation().Roll,
						ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().X,
						ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Y,
						ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Z,
						ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().W
				);
			
		}
		Row += "\n";
		GetInstance()->WriteToCondensedFile(Row);
	}
}

void USFLoggingBPLibrary::LogToIndividualFiles() {
	if (!GetInstance()->MasterNode)	return;
	
	for (auto& ActorLoggingInfo : GetInstance()->LoggingInfo) {
		if ((FDateTime::Now() - ActorLoggingInfo.LastTimeLogged).GetTotalMilliseconds() > ActorLoggingInfo.LogInMilliseconds || ActorLoggingInfo.LogInMilliseconds == 0) {
			ActorLoggingInfo.LastTimeLogged = FDateTime::Now();

			FString Out = 
				FString::Printf(TEXT("%i\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n"),
					GetInstance()->ProbandID,
					ActorLoggingInfo.ActorToLog->GetWorld()->GetTimeSeconds(),
					ActorLoggingInfo.ActorToLog->GetActorLocation().X,
					ActorLoggingInfo.ActorToLog->GetActorLocation().Y,
					ActorLoggingInfo.ActorToLog->GetActorLocation().Z,
					ActorLoggingInfo.ActorToLog->GetActorRotation().Pitch,
					ActorLoggingInfo.ActorToLog->GetActorRotation().Yaw,
					ActorLoggingInfo.ActorToLog->GetActorRotation().Roll,
					ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().X,
					ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Y,
					ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Z,
					ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().W
				);

			GetInstance()->WriteToIndividualFile(ActorLoggingInfo.IndividualFileHandle, Out, ActorLoggingInfo.IndividualActorFile);
		}
	}
}

/////--LogEvents--/////
void USFLoggingBPLibrary::LogCustomEvent(FString EventName) {
	if (!GetInstance()->MasterNode)	return;
	
	FString Out = FString::Printf(TEXT("%i\t%s\t%s\n"), 
			GetInstance()->ProbandID,
			*EventName,
			*FDateTime::Now().ToString());

	GetInstance()->WriteToEventLogsFile(Out);
}

void USFLoggingBPLibrary::LogCustomEventWithActor(FString EventName, AActor* Actor) {
	if (!GetInstance()->MasterNode) return;
	
	FString Out = 
		FString::Printf(TEXT("%i\t%s\t%s\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n"), 
			GetInstance()->ProbandID,
			*(Actor->GetFName().ToString()),
			*EventName,
			Actor->GetWorld()->GetTimeSeconds(),
			Actor->GetActorLocation().X,
			Actor->GetActorLocation().Y,
			Actor->GetActorLocation().Z,
			Actor->GetActorRotation().Pitch,
			Actor->GetActorRotation().Yaw,
			Actor->GetActorRotation().Roll,
			Actor->GetActorRotation().Quaternion().X,
			Actor->GetActorRotation().Quaternion().Y,
			Actor->GetActorRotation().Quaternion().Z,
			Actor->GetActorRotation().Quaternion().W
		);

	GetInstance()->WriteToActorEventFile(Out);
}

void USFLoggingBPLibrary::LogCustomEventWithActorAndName(FString EventName, AActor* Actor, FString ActorName) {
	if (!GetInstance()->MasterNode)	return;
	
	FString Out = 
		FString::Printf(TEXT("%i\t%s\t%s\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f"),
			GetInstance()->ProbandID,
			*ActorName,
			*EventName,
			Actor->GetWorld()->GetTimeSeconds(),
			Actor->GetActorLocation().X,
			Actor->GetActorLocation().Y,
			Actor->GetActorLocation().Z,
			Actor->GetActorRotation().Pitch,
			Actor->GetActorRotation().Yaw,
			Actor->GetActorRotation().Roll,
			Actor->GetActorRotation().Quaternion().X,
			Actor->GetActorRotation().Quaternion().Y,
			Actor->GetActorRotation().Quaternion().Z,
			Actor->GetActorRotation().Quaternion().W
		);

	GetInstance()->WriteToActorEventFile(Out);
}

UStudyLoggerSingleton* USFLoggingBPLibrary::GetInstance()
{
	static UStudyLoggerSingleton Instance;
	return &Instance;
}
