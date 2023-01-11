// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "SFStudySetup.h"
#include "SFParticipant.h"
#include "HUD/SFMasterHUD.h"
#include "GazeTracking/SFGazeTracker.h"

#include "Events/DisplayClusterEventWrapper.h"

#include "Widgets/SWindow.h"

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
	bool StartStudy();

	//Fade to the next condition (use this to proceed in the study once condition is done)
	//bForce also goes to the next condition if the current condition was not finished (not all required dependent variables gathered)
	UFUNCTION(BlueprintCallable)
	bool NextCondition(bool bForce = false);

	//This method can be used to jump to a specific condition (DO NOT USE during normal study run)
	//bForce also goes to the next condition if the current condition was not finished (not all required dependent variables gathered)
	UFUNCTION(BlueprintCallable)
	bool GoToCondition(const USFCondition* Condition, bool bForce = false);

	//Whether the study was started already
	UFUNCTION(BlueprintCallable)
	bool IsStarted() const;

	//the time that passed since the start of the study in seconds
	UFUNCTION(BlueprintCallable)
	float GetCurrentTime() const;

	//Get the level of a given factor in the current condition
	//use this to configure everything in the scene for this condition
	//Right Level is already accessible in BeginPlay
	UFUNCTION(BlueprintCallable)
	FString GetFactorLevel(FString FactorName) const;

	UFUNCTION(BlueprintCallable)
	FString GetCurrentPhase() const;

	UFUNCTION(BlueprintCallable)
	USFGazeTracker* GetGazeTracker() const;

	// ****************************************************************** // 
	// ******* Executing Study  (called by other parts of the framework)* //
	// ****************************************************************** //

	UFUNCTION()
	USFFadeHandler* GetFadeHandler();
	UFUNCTION()
	ASFStudySetup* GetStudySetup();

	UFUNCTION()
	void OnLevelLoaded();
	UFUNCTION()
	void OnFadedIn();
	UFUNCTION()
	USFParticipant* GetParticipant() const;


	// ****************************************************************** // 
	// *******      HUD      ******************************************** //
	// ****************************************************************** //

	//use this to print something to the log panel of the HUD, e.g., to inform the experimenter
	UFUNCTION(BlueprintCallable)
	void LogToHUD(FString Text);

	UFUNCTION()
	void UpdateHUD(FString Status);

	UFUNCTION(BlueprintCallable)
	ASFMasterHUD* GetHUD();

	UFUNCTION(BlueprintCallable)
	FExperimenterViewConfig GetExperimenterViewConfig() const;

	UFUNCTION(BlueprintCallable)
	USFExperimenterWindow* GetExperimenterWindow() const;

	//this is used by the SFMasterHUD to store content between levels
	FHUDSavedData HUDSavedData;

	// ****************************************************************** // 
	// *******    Logging    ******************************************** //
	// ****************************************************************** //

	UFUNCTION()
	USFLogObject* GetLogObject();

	//Log a comment (with a timestamp) to store events or user action etc. in the participant's log file
	UFUNCTION(BlueprintCallable, meta=(DeprecatedFunction, DeprecationMessage="Please use SFLoggingBPLibrary::LogComment instead"))
	void LogComment(const FString& Comment);

	//Log Data collected for a DependentVariable in this condition
	UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Please use SFLoggingBPLibrary::LogData instead"))
	void LogData(const FString& DependenVariableName, const FString& Value);

	//Is called once per tick, calls appropriate logging functions
	bool LogTick(float DeltaSeconds);

	//Needed to use core tick function
	UPROPERTY(BlueprintAssignable)
	FOnFadedInDelegate OnFadedInDelegate;
	FDelegateHandle TickDelegateHandle;

protected:

	void EndStudy();

	void GoToConditionSynced(FString ConditionName, bool bForced);
	DECLARE_DISPLAY_CLUSTER_EVENT(USFGameInstance, GoToConditionSynced);

	void RestoreLastParticipantForDebugStart(USFCondition* InStartCondition);
	//method called by a timer if we want to directly fade in on startup
	void StartFadingIn();
	FTimerHandle StartFadingTimerHandle;

	void InitFadeHandler(FFadeConfig FadeConfig);

	void PrepareWithStudySetup(ASFStudySetup* Setup);

	UFUNCTION(BlueprintCallable)
	void OnWorldStart();

	UPROPERTY()
	USFParticipant* Participant = nullptr;


	UPROPERTY()
	USFFadeHandler* FadeHandler = nullptr;
	UPROPERTY()
	ASFStudySetup* StudySetup;

	UPROPERTY()
	USFExperimenterWindow* ExperimenterWindow = nullptr;
	FExperimenterViewConfig ExperimenterViewConfig;

	// this is used if the study should not be started from the beginning
	// e.g., when starting a map directly for debugging in the editor
	// or when continuing a crashed study run
	USFCondition* StartCondition;

	// State of Study / GameInstance
	UPROPERTY()
	bool bStudyStarted = false;

	//singleton object of this class, to easier access it
	static USFGameInstance* Instance;

	// this is only needed if a map should be started for debugging, but Init wasn't called
	UPROPERTY()
	USFCondition* ConditionToStartAtInit=nullptr;

	bool bStartedOnUnrelatedMap = false;

	UPROPERTY()
	USFGazeTracker* GazeTracker;

	//Controls central logging functionality, stores logging parameters
	UPROPERTY()
	USFLogObject* LogObject;
};

