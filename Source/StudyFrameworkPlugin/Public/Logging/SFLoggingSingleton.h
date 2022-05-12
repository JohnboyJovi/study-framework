// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"

struct FLoggingInfo
{	
	int32 LogInMilliseconds = 0;
	AActor* ActorToLog = nullptr;
	FString ActorLogName = "";
	FDateTime LastTimeLogged = NULL;

	FString IndividualActorFile = "";
	TUniquePtr<IFileHandle> IndividualFileHandle = nullptr;
	
	FLoggingInfo(int32 LogTimer, AActor* Actor, FString LogName, FString IndividualDir)
	{
		this->LogInMilliseconds = LogTimer;
		this->ActorToLog = Actor;
		this->ActorLogName = LogName;
		IndividualActorFile = IndividualDir + "/" + LogName + ".txt";
		LastTimeLogged = FDateTime::Now();
		IndividualFileHandle = nullptr;
	}
	FLoggingInfo(){}
};

class LOGGINGPLUGIN_API USFLoggingSingleton{
private:
	
	FString EventLogsFileHeader = "ProbandID\tEvent\tTriggeredAt\n";
	FString ActorEventFileHeader = "ProbandID\tActor\tEvent\tTime\tActorXPos\tActorYPos\tActorZPos\tActorPitchOri\tActorYawOri\tActorRollOri\tActorQuatX\tActorQuatY\tActorQuatZ\tActorQuatW\n";
	FString IndividualFileHeader = "ProbandID\tTime\tActorXPos\tActorYPos\tActorZPos\tActorPitchOri\tActorYawOri\tActorRollOri\tActorQuatX\tActorQuatY\tActorQuatZ\tActorQuatW\n";
	
public:
	USFLoggingSingleton();

	TArray<FLoggingInfo> LoggingInfo;
	FDateTime StaticDateTime;
	FString LogDir = "";
	bool MasterNode = false;
	int32 ProbandID = -1;

	FString CondensedActorFile = "";
	FString IndividualActorsDir = "";
	FString CustomEventsFile = "";
	FString ActorEventsFile = "";
	TUniquePtr<IFileHandle> CustomEventFileHandle = nullptr;
	TUniquePtr<IFileHandle> CondensedFileHandle = nullptr;
	TUniquePtr<IFileHandle> ActorEventFileHandle = nullptr;

	void AddActor(AActor* Actor, int32 LogTimer);
	void AddActorWithName(AActor * Actor, int32 LogTimer, FString LogName);
	void AddActors(TArray<AActor*>, int32 LogTimer);
	void Initialize();
	void Destruct();
	void Initialize(FString FilePath);
	void Initialize(uint32 ProbandIDval);
	void SetUpLogDir();
	void SetUpProbandID(FString FilePath);

	void WriteToCondensedFile(FString &String);
	void WriteToIndividualFile(TUniquePtr<IFileHandle> & IndividualFileHandle, FString & String, FString & ActorFile);
	void WriteToFile(TUniquePtr<IFileHandle> & IndividualFileHandle, FString & String, FString & ActorFile, FString & Header);
	void WriteToEventLogsFile(FString & String);
	void WriteToActorEventFile(FString & String);
};
