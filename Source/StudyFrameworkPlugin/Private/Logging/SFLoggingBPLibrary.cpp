// Fill out your copyright notice in the Description page of Project Settings.


#include "Logging/SFLoggingBPLibrary.h"

#include "SFGameInstance.h"
 #include "SFPlugin.h"
 #include "Logging/SFLogObject.h"
 #include "Misc/FileHelper.h"

 void USFLoggingBPLibrary::LogToFile() {
     UE_LOG(LogTemp, Display, TEXT("Called LogToFileFunction!"));
    if(!USFGameInstance::Get() || !USFGameInstance::Get()->GetStudySetup())
    {
        return;
    }
    const USFLogObject* Instance = USFGameInstance::Get()->GetStudySetup()->LogObject;
 	if (Instance->LogThis == false) {
 		return;
 	}
// 	for (auto& ActorLoggingInfo : SFPluginModule.GetInstance()->LoggingInfo) {
// 		if (ActorLoggingInfo.LogNextTick == true) {
// 			ActorLoggingInfo.LogNextTick = false;
// 			float TimeSinceStart = ActorLoggingInfo.ActorToLog->GetWorld()->GetTimeSeconds();
// 			FString TimeSinceStartString = FString::SanitizeFloat(TimeSinceStart);
// 			FString out = "" + TimeSinceStartString +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().X) +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Y) +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Z) +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Pitch) +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Yaw) +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Roll) +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().X) +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Y) +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Z) +
// 				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().W) +
// 				"\n";
// 			FString DirActor = Instance->LogDir + "/Actors";
// 			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
// 			if (PlatformFile.CreateDirectoryTree(*DirActor)) {
// 				FString AbsPath = DirActor + "/" + ActorLoggingInfo.LogName + ".txt";
// 				if (!PlatformFile.FileExists(*AbsPath)) {
// 					FString FileHeader = "Time\tActorXPos\tActorYPos\tActorZPos\tActorPitchOri\tActorYawOri\tActorRollOri\tActorQuatX\tActorQuatY\tActorQuatZ\tActorQuatW\n";
// 					FFileHelper::SaveStringToFile(FileHeader, *AbsPath);
// 				}
// 				FFileHelper::SaveStringToFile(out, *AbsPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
// 			}
// 		}
// 		if (((FDateTime::Now() - ActorLoggingInfo.TimeStorage).GetTotalMilliseconds() > ActorLoggingInfo.LogTimer) || (ActorLoggingInfo.LogTimer == 0)) {
// 			ActorLoggingInfo.TimeStorage = FDateTime::Now();
// 			ActorLoggingInfo.LogNextTick = true;
// 		}
// 	}
     UE_LOG(LogTemp, Display, TEXT("Completed LogToFileFunction!"));
 }
 void USFLoggingBPLibrary::AddActor(AActor* Actor, int32 LogTimer, FString LogName)
 {
     if (USFGameInstance::Get() && USFGameInstance::Get()->GetStudySetup())
     {
         USFGameInstance::Get()->GetStudySetup()->LogObject->AddActorWithName(Actor, LogTimer, LogName);
     }
 }