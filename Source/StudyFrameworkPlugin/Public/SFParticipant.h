// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Dom/JsonObject.h"
#include "SFStudySetup.h"
#include "SFStudyPhase.h"
#include "SFCondition.h"


#include "SFParticipant.generated.h"

class USFLogger;
class USFGameInstance;


UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFParticipant : public UObject
{
	GENERATED_BODY()

	friend USFGameInstance;
	//so it can call SetCondition() which should only be called through the GameInstance which is the central interface

public:
	USFParticipant();
	~USFParticipant();

	bool Initialize(int Participant, FString JsonFilePath, FString LogName = "NormalLog",
	                FString SaveDataLogName = "SaveLog");

	bool StartStudy(USFStudySetup* StudySetup);
	void EndStudy(); // TODO implement Participant::EndStudy()

	void SaveDataArray(FString Where, TArray<FString> Data);
	void LogData(FString Data);
	void CommitData(); // TODO need CommitData()?


	USFCondition* GetCurrentCondition() const;
	USFCondition* GetNextCondition() const;
	const TArray<USFCondition*> GetAllConditions() const;
	int GetID() const;

	// this method can be used to recover from a crash during the study
	// (or when directly starting an intermediate map for testing)
	static TArray<USFCondition*> GetLastParticipantsConditions();
	static int GetLastParticipantId();


protected:
	bool SetCondition(const USFCondition* NextCondition);

	//this logs the current participant and state into a file, for recovery etc.
	void LogCurrentParticipant() const;

	void GenerateExecutionJsonFile() const;
	static TArray<USFCondition*> ReadExecutionJsonFile(int ParticipantID);

	UPROPERTY()
	int ParticipantID;

	UPROPERTY()
	USFLogger* Logger;


	UPROPERTY()
	USFStudySetup* StudySetup;
	UPROPERTY()
	TArray<USFCondition*> Conditions;
	UPROPERTY()
	int CurrentConditionIdx;
};
