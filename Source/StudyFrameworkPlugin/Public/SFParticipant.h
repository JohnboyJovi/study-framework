// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Dom/JsonObject.h"
#include "SFStudySetup.h"
#include "SFStudyPhase.h"
#include "SFCondition.h"
#include "SFMultipleTrialDependentVariable.h"


#include "SFParticipant.generated.h"


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

	bool Initialize(int ParticipantSequenceNumber, FString ParticipantID);
	void SetStudyConditions(TArray<USFCondition*> NewConditions);

	bool StartStudy();
	void EndStudy();

	USFCondition* GetCurrentCondition() const;
	USFCondition* GetNextCondition() const;
	const TArray<USFCondition*> GetAllConditions() const;
	int GetCurrentConditionNumber() const;
	int GetSequenceNumber() const;
	FString GetID() const;

	static bool WasParticipantIdAlreadyUsed(FString NewParticipantID);

	// this method can be used to recover from a crash during the study
	// (or when directly starting an intermediate map for testing)
	static TArray<USFCondition*> GetLastParticipantsConditions();
	static int GetLastParticipantSequenceNumber();
	static FString GetLastParticipantID();
	static int GetLastParticipantLastConditionStarted();
	static TMap<FString, bool> GetLastParticipantHasStartedConditionValues();
	static bool GetLastParticipantFinished();
	static ASFStudySetup* GetLastParticipantSetup();
	void LoadLastParticipantsIndependentVariables();

	const FString& GetParticipantLoggingInfix() const;

	const TPair<USFIndependentVariable*, FString> GetIndependentVariable(const FString& VarName);

	UFUNCTION(BlueprintCallable)
	void SetIndependentVariableValue(const FString& VarName, const FString& Value);
	void SetIndependentVariablesFromStudySetup(ASFStudySetup* Setup);

	FString GetCurrentTimeAsString() const;
	float GetCurrentTime() const;

	bool LoadConditionsFromJson();
	void RecoverStudyResultsOfFinishedConditions();


	// The results of all participants are stored in a file per phase (called longtable)
	// for the data to be ready to use in statistics software. This methods clears all
	// of that data (e.g. if study is entirely restarted).
	// The data can be recovered from /StudyLogs/RecyclingBin
	static void ClearPhaseLongtables(ASFStudySetup* StudySetup);

	// Whenever we delete data in a file (e.g. with by restarting a condition),
	// we want to create a backup, to enable data recovery
	// EditOperation reflects the reason (e.g. restart condition), which appears in
	// recycling bin file name
	void CreateLongTableBackUp(const FString PathToSrcFile) const;
	void StoreTrialInTrialDVLongTable(USFMultipleTrialDependentVariable* DependentVariable, TArray<FString> Values) const;
	void DeleteStoredDataForConditionFromLongTable(USFCondition* Condition);
	void DeleteStoredTrialDataForCondition(USFCondition* Condition, USFMultipleTrialDependentVariable* DependentVariable);
	void SetCurrentBackUpFolderName(FString BackUpFolderName);

protected:
	bool SetCondition(const USFCondition* NextCondition);

	//this logs the current participant and state into a file, for recovery etc.
	void LogCurrentParticipant() const;

	void GenerateExecutionJsonFile() const;
	void UpdateIndependentVarsExecutionJsonFile() const;
	static void ReadExecutionJsonFile(FString ParticipantID, TArray<USFCondition*>& Conditions_Out, TMap<USFIndependentVariable*, FString>& IndependentVariablesValues_Out);

	void StoreInPhaseLongTable() const;
	void StoreInIndependentVarLongTable() const;
	void RemoveLinesOfConditionAndWriteToFile(USFCondition* Condition, FString Filename);

	// This is the parent folder within RecyclingBin, where backups
	// of the current operation will be stored.
	// E.g. "RestartConditionBackUp-TIMESTAMP"
	// Whenever we delete lines within a .csv file we should update this.
	// We want to store all files related to one operation in the same
	// backup folder, this is why we need this variable.
	FString CurrentBackUpFolderName;
	FString ParticipantID;
	//sequence number is used for randomization etc. it is also unique per participant and starts at 0
	int ParticipantSequenceNumber; 

	UPROPERTY()
	TArray<USFCondition*> Conditions;

	UPROPERTY(BlueprintReadOnly)
	TMap<USFIndependentVariable*, FString> IndependentVariablesValues;

	UPROPERTY()
	int CurrentConditionIdx;

	double StartTime = 0.0;

	FString ParticipantLoggingInfix;
};
