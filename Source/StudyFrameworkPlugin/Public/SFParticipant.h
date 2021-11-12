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

public:
	USFParticipant();
	~USFParticipant();

	bool Initialize(FString IdNew, FString JsonFilePath, FString LogName = "NormalLog",
	                FString SaveDataLogName = "SaveLog");

	void GenerateExecutionJsonFile() const; // TODO implement GenerateExecutionJsonFile()

	bool StartStudy(USFStudySetup* StudySetup);
	FString NextCondition(); //returns the level to load
	void EndStudy(); // TODO implement Participant::EndStudy()

	void SaveDataArray(FString Where, TArray<FString> Data);
	void LogData(FString Data);
	void CommitData(); // TODO need CommitData()?


	const USFCondition* GetCurrentCondition() const;
	const TArray<USFCondition*> GetAllConditions() const;
	FString GetID() const;


protected:
	UPROPERTY()
	FString ParticipantID;

	UPROPERTY()
	USFLogger* Logger;


	UPROPERTY()
	USFStudySetup* StudySetup;
	UPROPERTY()
	TArray<USFCondition*> Conditions;
	UPROPERTY()
	int CurrentConditionIdx;
};
