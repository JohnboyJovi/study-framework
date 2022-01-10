// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SFParticipant.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SFStudySetup.h"
#include "HUD/SFMasterHUD.h"

#include "Events/DisplayClusterEventWrapper.h"

#include "SFGameInstance.generated.h"

class USFFadeHandler;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFadedInDelegate);

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFGameInstance : public UGameInstance
{
	GENERATED_BODY()


public: 

	//override UGameInstance methods
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;


	// getter for the GameInstance itself
	UFUNCTION(BlueprintCallable,  meta = (DisplayName="Get SFGameInstance"))
	static USFGameInstance* Get();
	// a check method whether Init was called by the UE system, otherwise Get() cannot be called
	static bool IsInitialized();

	// ****************************************************************** // 
	// ******* Control Study ******************************************** //
	// ****************************************************************** //

	//Start the study
	UFUNCTION(BlueprintCallable)
	bool StartStudy(const USFCondition* StartCondition = nullptr);

	//End the study (called internally, not sure whether this shoul be used from outside)
	UFUNCTION(BlueprintCallable)
	void EndStudy();

	//Fade to the next condition (use this to proceed in the study once condition is done)
	UFUNCTION(BlueprintCallable)
	bool NextCondition();

	//This method can be used to jump to a specific condition (DO NOT USE during normal study run)
	UFUNCTION(BlueprintCallable)
	bool GoToCondition(const USFCondition* Condition);

	//Whether the study was started already
	UFUNCTION(BlueprintCallable)
	bool IsStarted() const;

	//Get the level of a given factor in the current condition
	//use this to configure everything in the scene for this condition
	//Right Level is already accessible in BeginPlay
	UFUNCTION(BlueprintCallable)
	FString GetFactorLevel(FString FactorName) const;

	//Log Data collected for a DependentVariable in this condition
	UFUNCTION(BlueprintCallable)
	void LogData(const FString& DependenVariableName, const FString& Value);

	//Log a comment (with a timestamp) to store events or user action etc. in the participant's log file
	UFUNCTION(BlueprintCallable)
	void LogComment(const FString& Comment);

	//use this to print something to the log panel of the HUD, e.g., to inform the experimenter
	UFUNCTION()
	void LogToHUD(FString Text);

	//A delegate to register to when you want to start behavior once the condition is faded in
	//NOTE: BeginPlay is called while it is still faded out!
	UPROPERTY(BlueprintAssignable)
	FOnFadedInDelegate OnFadedInDelegate;


	// ****************************************************************** // 
	// ******* Executing Study  (called by other parts of the framework)* //
	// ****************************************************************** //

	UFUNCTION()
	USFFadeHandler* GetFadeHandler();
	UFUNCTION()
	ASFStudySetup* GetStudySetup();


	UFUNCTION()
	void SpawnAllActorsForLevel();
	UFUNCTION()
	void OnLevelLoaded();
	UFUNCTION()
	void OnFadedIn();
	UFUNCTION()
	USFParticipant* GetParticipant() const;


	// ****************************************************************** // 
	// *******      HUD      ******************************************** //
	// ****************************************************************** //

	UFUNCTION()
	void UpdateHUD(FString Status);

	//this is used by the SFMasterHUD to store content between levels
	FHUDSavedData HUDSavedData;

protected:

	void GoToConditionSynced(FString ConditionName);
	DECLARE_DISPLAY_CLUSTER_EVENT(USFGameInstance, GoToConditionSynced);

	void RestoreLastParticipant(USFCondition* StartCondition);
	//method called by a timer if we want to directly fade in on startup
	void StartFadingIn();
	FTimerHandle StartFadingTimerHandle;

	void InitFadeHandler(FFadeConfig FadeConfig);

	void PrepareWithStudySetup(ASFStudySetup* Setup);

	void GenerateTestStudyRuns(int NrOfRuns) const;

	UPROPERTY()
	USFParticipant* Participant = nullptr;


	UPROPERTY()
	USFFadeHandler* FadeHandler = nullptr;
	UPROPERTY()
	ASFStudySetup* StudySetup;

	// State of Study / GameInstance
	UPROPERTY()
	bool bStudyStarted = false;

	//singleton object of this class, to easier access it
	static USFGameInstance* Instance;

	// this is only needed if a map should be started for debugging, but Init wasn't called
	USFCondition* ConditionToStartAtInit=nullptr;
};

