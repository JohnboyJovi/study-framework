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

	bool Initialize(FString IdNew, FString JsonFilePath, FString LogName = "NormalLog",
	                FString SaveDataLogName = "SaveLog");

	void GenerateExecutionJsonFile() const;

	bool StartStudy(USFStudySetup* StudySetup);
	USFCondition* NextCondition();
	void EndStudy(); // TODO implement Participant::EndStudy()

	void SaveDataArray(FString Where, TArray<FString> Data);
	void LogData(FString Data);
	void CommitData(); // TODO need CommitData()?


	USFCondition* GetCurrentCondition() const;
	const TArray<USFCondition*> GetAllConditions() const;
	FString GetID() const;


protected:
	bool SetCondition(const USFCondition* NextCondition);

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
