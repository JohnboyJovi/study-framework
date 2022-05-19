// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SFLoggingBPLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "IDisplayCluster.h"
#include "SFPlugin.h"

USFLoggingBPLibrary::USFLoggingBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
}

void USFLoggingBPLibrary::LogToOneFile(int32 Timer) {
	if (!(FModuleManager::Get().IsModuleLoaded("StudyFrameworkPlugin"))) {
		return;
	}
	FSFPlugin& SFPluginModule = FModuleManager::GetModuleChecked<FSFPlugin>("StudyFrameworkPlugin");
	USFLogObject* Instance = SFPluginModule.GetInstance();
	if (Instance->LogThis == false) {
		return;
	}
	if ((FDateTime::Now() - Instance->StaticDateTime).GetTotalMilliseconds() > Timer || Timer == 0) {
		Instance->StaticDateTime = FDateTime::Now();
		bool FirstActor = true;
		FString Row = "";
		for (auto& ActorLoggingInfo : SFPluginModule.GetInstance()->LoggingInfo) {
			if (FirstActor == true) {
				FirstActor = false;
				float TimeSinceStart = ActorLoggingInfo.ActorToLog->GetWorld()->GetTimeSeconds();
				Row = "" + FString::Printf(TEXT("%.3f"), TimeSinceStart) +
					"\t" + ActorLoggingInfo.LogName +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().X) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Y) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Z) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Pitch) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Yaw) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Roll) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().X) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Y) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Z) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().W);
			}
			else {
				Row = Row +
					"\t" + ActorLoggingInfo.LogName +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().X) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Y) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Z) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Pitch) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Yaw) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Roll) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().X) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Y) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Z) +
					"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().W);
			}
		}
		Row = Row + "\n";
		FString DirActor = Instance->LogDir + "/Actors";
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.CreateDirectoryTree(*DirActor)) {
			FString AbsPath = DirActor + "/" + "AllActorLogs.txt";
			if (!PlatformFile.FileExists(*AbsPath)) {
				bool FirstHeader = true;
				FString FileHeader = "";
				for (int i = 0; i < Instance->LoggingInfo.Num(); i++) {
					if (FirstHeader == true) {
						FileHeader = FileHeader + "Time\tActorName\tActorXPos\tActorYPos\tActorZPos\tActorPitchOri\tActorYawOri\tActorRollOri\tActorQuatX\tActorQuatY\tActorQuatZ\tActorQuatW";
						FirstHeader = false;
					}
					else {
						FileHeader = FileHeader + "\tActorName\tActorXPos\tActorYPos\tActorZPos\tActorPitchOri\tActorYawOri\tActorRollOri\tActorQuatX\tActorQuatY\tActorQuatZ\tActorQuatW";
					}
				}
				FileHeader = FileHeader + "\n";
				FFileHelper::SaveStringToFile(FileHeader, *AbsPath);
			}
			FFileHelper::SaveStringToFile(Row, *AbsPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
		}
	}
}

void USFLoggingBPLibrary::LogToFile() {
	if (!(FModuleManager::Get().IsModuleLoaded("StudyFrameworkPlugin"))) {
		return;
	}
	FSFPlugin& SFPluginModule = FModuleManager::GetModuleChecked<FSFPlugin>("StudyFrameworkPlugin");
	USFLogObject* Instance = SFPluginModule.GetInstance();
	if (Instance->LogThis == false) {
		return;
	}
	for (auto& ActorLoggingInfo : SFPluginModule.GetInstance()->LoggingInfo) {
		if (ActorLoggingInfo.LogNextTick == true) {
			ActorLoggingInfo.LogNextTick = false;
			float TimeSinceStart = ActorLoggingInfo.ActorToLog->GetWorld()->GetTimeSeconds();
			FString TimeSinceStartString = FString::SanitizeFloat(TimeSinceStart);
			FString out = "" + TimeSinceStartString +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().X) +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Y) +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Z) +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Pitch) +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Yaw) +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Roll) +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().X) +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Y) +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().Z) +
				"\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Quaternion().W) +
				"\n";
			FString DirActor = Instance->LogDir + "/Actors";
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			if (PlatformFile.CreateDirectoryTree(*DirActor)) {
				FString AbsPath = DirActor + "/" + ActorLoggingInfo.LogName + ".txt";
				if (!PlatformFile.FileExists(*AbsPath)) {
					FString FileHeader = "Time\tActorXPos\tActorYPos\tActorZPos\tActorPitchOri\tActorYawOri\tActorRollOri\tActorQuatX\tActorQuatY\tActorQuatZ\tActorQuatW\n";
					FFileHelper::SaveStringToFile(FileHeader, *AbsPath);
				}
				FFileHelper::SaveStringToFile(out, *AbsPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
			}
		}
		if (((FDateTime::Now() - ActorLoggingInfo.TimeStorage).GetTotalMilliseconds() > ActorLoggingInfo.LogTimer) || (ActorLoggingInfo.LogTimer == 0)) {
			ActorLoggingInfo.TimeStorage = FDateTime::Now();
			ActorLoggingInfo.LogNextTick = true;
		}
	}
}

void USFLoggingBPLibrary::AddActorToLog(AActor* Actor, int32 LogTimer) {
	if (FModuleManager::Get().IsModuleLoaded("StudyFrameworkPlugin")) {
		FSFPlugin& SFPluginModule = FModuleManager::GetModuleChecked<FSFPlugin>("StudyFrameworkPlugin");
		if (SFPluginModule.GetInstance()->LogThis == false) {
			return;
		}
		SFPluginModule.GetInstance()->AddActor(Actor, (++LogTimer));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Es ist ein Fehler aufgetreten - Der Actor konnte nicht hinzugefuegt werden!"));
	}
}

void USFLoggingBPLibrary::AddActorToLogWithName(AActor* Actor, int32 LogTimer, FString LogName) {
	if (FModuleManager::Get().IsModuleLoaded("StudyFrameworkPlugin")) {
		FSFPlugin& SFPluginModule = FModuleManager::GetModuleChecked<FSFPlugin>("StudyFrameworkPlugin");
		if (SFPluginModule.GetInstance()->LogThis == false) {
			return;
		}
		SFPluginModule.GetInstance()->AddActorWithName(Actor, (++LogTimer), LogName);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Es ist ein Fehler aufgetreten - Der Actor konnte nicht hinzugefuegt werden!"));
	}
}

void USFLoggingBPLibrary::AddActorsToLog(TArray<AActor*> ActorArray, int32 LogTimer) {
	if (FModuleManager::Get().IsModuleLoaded("StudyFrameworkPlugin")) {
		FSFPlugin& SFPluginModule = FModuleManager::GetModuleChecked<FSFPlugin>("StudyFrameworkPlugin");
		if (SFPluginModule.GetInstance()->LogThis == false) {
			return;
		}
		SFPluginModule.GetInstance()->AddActors(ActorArray, (++LogTimer));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Es ist ein Fehler aufgetreten - Der Actor konnte nicht hinzugefuegt werden!"));
	}
}

void USFLoggingBPLibrary::Initialize() {
	if (FModuleManager::Get().IsModuleLoaded("StudyFrameworkPlugin")) {
		FSFPlugin& SFPluginModule = FModuleManager::GetModuleChecked<FSFPlugin>("StudyFrameworkPlugin");
		SFPluginModule.GetInstance()->Initialize();
	}
}

void USFLoggingBPLibrary::LogCustomEvent(FString EventName) {
	if (!(FModuleManager::Get().IsModuleLoaded("StudyFrameworkPlugin"))) {
		return;
	}
	FSFPlugin& SFPluginModule = FModuleManager::GetModuleChecked<FSFPlugin>("StudyFrameworkPlugin");
	USFLogObject* Instance = SFPluginModule.GetInstance();
	if (Instance->LogThis == false) {
		return;
	}
	FString out = EventName + "\t" + FDateTime::Now().ToString() + "\n";
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*Instance->LogDir)) {
		FString AbsPath = Instance->LogDir + "/" + "EventLogs.txt";
		if (!PlatformFile.FileExists(*AbsPath)) {
			FString FirstRow = "Event\tTriggeredAt\n";
			FFileHelper::SaveStringToFile(FirstRow, *AbsPath);
		}
		else {
			FFileHelper::SaveStringToFile(out, *AbsPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
		}
	}
}

void USFLoggingBPLibrary::LogCustomEventWithActor(FString EventName, AActor* Actor) {
	if (!(FModuleManager::Get().IsModuleLoaded("StudyFrameworkPlugin"))) {
		return;
	}
	FSFPlugin& SFPluginModule = FModuleManager::GetModuleChecked<FSFPlugin>("StudyFrameworkPlugin");
	USFLogObject* Instance = SFPluginModule.GetInstance();
	if (Instance->LogThis == false) {
		return;
	}
	FString out = Actor->GetFName().ToString() +
		"\t" + EventName +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetWorld()->GetTimeSeconds()) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorLocation().X) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorLocation().Y) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorLocation().Z) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Pitch) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Yaw) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Roll) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Quaternion().X) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Quaternion().Y) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Quaternion().Z) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Quaternion().W) + "\n";
	FString Header = "Actor\tEvent\tTime\tActorXPos\tActorYPos\tActorZPos\tActorPitchOri\tActorYawOri\tActorRollOri\tActorQuatX\tActorQuatY\tActorQuatZ\tActorQuatW\n";
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*Instance->LogDir)) {
		FString AbsPath = Instance->LogDir + "/" + "ActorEventLogs.txt";
		if (!PlatformFile.FileExists(*AbsPath)) {
			FFileHelper::SaveStringToFile(Header, *AbsPath);
		}
		FFileHelper::SaveStringToFile(out, *AbsPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}
}

void USFLoggingBPLibrary::LogCustomEventWithActorAndName(FString EventName, AActor* Actor, FString ActorName) {
	if (!(FModuleManager::Get().IsModuleLoaded("StudyFrameworkPlugin"))) {
		return;
	}
	FSFPlugin& SFPluginModule = FModuleManager::GetModuleChecked<FSFPlugin>("StudyFrameworkPlugin");
	USFLogObject* Instance = SFPluginModule.GetInstance();
	if (Instance->LogThis == false) {
		return;
	}
	FString out = ActorName +
		"\t" + EventName +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetWorld()->GetTimeSeconds()) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorLocation().X) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorLocation().Y) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorLocation().Z) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Pitch) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Yaw) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Roll) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Quaternion().X) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Quaternion().Y) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Quaternion().Z) +
		"\t" + FString::Printf(TEXT("%.3f"), Actor->GetActorRotation().Quaternion().W) + "\n";
	FString Header = "Actor\tEvent\tTime\tActorXPos\tActorYPos\tActorZPos\tActorPitchOri\tActorYawOri\tActorRollOri\tActorQuatX\tActorQuatY\tActorQuatZ\tActorQuatW\n";
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*Instance->LogDir)) {
		FString AbsPath = Instance->LogDir + "/" + "ActorEventLogs.txt";
		if (!PlatformFile.FileExists(*AbsPath)) {
			FFileHelper::SaveStringToFile(Header, *AbsPath);
		}
		FFileHelper::SaveStringToFile(out, *AbsPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}
}