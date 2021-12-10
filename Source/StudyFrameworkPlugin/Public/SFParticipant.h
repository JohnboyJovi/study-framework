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

	bool Initialize(int Participant);

	bool StartStudy(USFStudySetup* StudySetup);
	void EndStudy(); // TODO implement Participant::EndStudy()

	void LogData(const FString& DependentVariableName, const FString& Value);
	void LogComment(const FString& Comment);


	USFCondition* GetCurrentCondition() const;
	USFCondition* GetNextCondition() const;
	const TArray<USFCondition*> GetAllConditions() const;
	int GetID() const;

	// this method can be used to recover from a crash during the study
	// (or when directly starting an intermediate map for testing)
	static TArray<USFCondition*> GetLastParticipantsConditions();
	static int GetLastParticipantId();
	bool LoadConditionsFromJson();


protected:
	bool SetCondition(const USFCondition* NextCondition);

	//this logs the current participant and state into a file, for recovery etc.
	void LogCurrentParticipant() const;

	void GenerateExecutionJsonFile() const;
	static TArray<USFCondition*> ReadExecutionJsonFile(int ParticipantID);

	void StoreInPhaseLongTable() const;
	FString GetCurrentTime() const;

	UPROPERTY()
	int ParticipantID;

	UPROPERTY()
	TArray<USFCondition*> Conditions;

	UPROPERTY()
	int CurrentConditionIdx;

	double StartTime = 0.0;
};
