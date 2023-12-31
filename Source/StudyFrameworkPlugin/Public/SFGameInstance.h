// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "SFStudySetup.h"
#include "SFParticipant.h"
#include "HUD/SFMasterHUD.h"
#include "HUD/SFFadeHandler.h"
#include "GazeTracking/SFGazeTracker.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "Cluster/DisplayClusterClusterEvent.h"

#include "Widgets/SWindow.h"

#include "SFGameInstance.generated.h"


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
	//Fade: if set to AsDefault, the value specified in ASFStudySetup::bNoFadingOnSameMap considered to decide whether to fade btw 2 conditions on the same map
	//      if ForceNoFade: independently from ASFStudySetup::bNoFadingOnSameMap it is not faded btw conditions on the same map
	//      if ForceFade: independently from ASFStudySetup::bNoFadingOnSameMap it is always faded
	UFUNCTION(BlueprintCallable)
	bool NextCondition(bool bForce = false, EFadeBetweenCondition Fade = EFadeBetweenCondition::AsDefault);

	//This method can be used to jump to a specific condition (DO NOT USE during normal study run)
	//bForce also goes to the next condition if the current condition was not finished (not all required dependent variables gathered)
	//Fade: if set to AsDefault, the value specified in ASFStudySetup::bNoFadingOnSameMap considered to decide whether to fade btw 2 conditions on the same map
	//      if ForceNoFade: independently from ASFStudySetup::bNoFadingOnSameMap it is not faded btw conditions on the same map
	//      if ForceFade: independently from ASFStudySetup::bNoFadingOnSameMap it is always faded
	UFUNCTION(BlueprintCallable)
	bool GoToCondition(const USFCondition* Condition, bool bForce = false, EFadeBetweenCondition Fade = EFadeBetweenCondition::AsDefault);

	//Whether the study was started already
	UFUNCTION(BlueprintCallable)
	bool IsStarted() const;

	//Whether the study is done (this is actually only true for a short duration), as once faded out Unreal is quit
	UFUNCTION(BlueprintCallable)
	bool HasEnded() const;

	

	//the time that passed since the start of the study in seconds
	UFUNCTION(BlueprintCallable)
	float GetCurrentTime() const;

	//Get the level of a given factor in the current condition
	//use this to configure everything in the scene for this condition
	//Right Level is already accessible in BeginPlay
	//return "FactorNotPresent" and prints an error if factor does not exist
	UFUNCTION(BlueprintCallable)
	FString GetFactorLevel(FString FactorName) const;

	//Returns whether a factor with the given name is available in the current condition
	//calling GetFactorLevel() directly will also work for non-existing factors
	//but will also print an error
	UFUNCTION(BlueprintCallable)
	bool HasFactor(FString FactorName) const;

	//Get value reported for an independet variable or "" if none was recorded
	UFUNCTION(BlueprintCallable)
	FString GetIndependentVariableValue(FString VariableName) const;

	UFUNCTION(BlueprintCallable)
	FString GetCurrentPhase() const;

	//returns the number of this condition (which is also shown in the HUD), starting from 1 going to # of Conditions
	//returns -1 if something goes wrong (e.g., participant is not initialized)
	UFUNCTION(BlueprintCallable)
	int GetCurrentConditionsSequenceNumber() const;

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
	UFUNCTION(BlueprintCallable)
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

	//setting HUD in-/visibile at runtime
	UFUNCTION(BlueprintCallable)
	void SetHUDVisible(bool bVisible);

	//this is used by the SFMasterHUD to store content between levels
	FHUDSavedData HUDSavedData;

	// ****************************************************************** // 
	// *******    Logging    ******************************************** //
	// ****************************************************************** //

	UFUNCTION()
	USFLogObject* GetLogObject();

	//Log a comment (with a timestamp) to store events or user action etc. in the participant's log file
	UFUNCTION(BlueprintCallable)
	void LogComment(const FString& Comment, bool bAlsoLogToHUD = true);

	//Log Data collected for a SFDependentVariable in this condition
	UFUNCTION(BlueprintCallable)
	void LogData(const FString& DependentVariableName, const FString& Value);

	//Log Data collected for a SFMultipleTrialDependentVariable in this condition
	UFUNCTION(BlueprintCallable)
	void LogTrialData(const FString& DependentVariableName, const TArray<FString>& Values);

	//Is called once per tick, calls appropriate logging functions
	bool LogTick(float DeltaSeconds);

	//Needed to use core tick function
	UPROPERTY(BlueprintAssignable)
	FOnFadedInDelegate OnFadedInDelegate;
	FDelegateHandle TickDelegateHandle;

protected:

	void EndStudy();

	//we use cluster events so GoToConditionSynced can not run out of sync when using nDisplay in cluster mode
	void GoToConditionSynced(FString ConditionName, bool bForced, EFadeBetweenCondition Fade); //send the cluster event
	void HandleGoToConditionSynced(FString ConditionName, bool bForced, EFadeBetweenCondition Fade); //process the cluter event
	FOnClusterEventJsonListener ClusterEventListenerDelegate;
	void HandleClusterEvent(const FDisplayClusterClusterEventJson& Event);


	void RestoreLastParticipantForDebugStart(USFCondition* InStartCondition);
	//method called by a timer if we want to directly fade in on startup
	void StartFadingIn();
	FTimerHandle StartFadingTimerHandle;

	void InitFadeHandler(FFadeConfig FadeConfig);

	void PrepareWithStudySetup(ASFStudySetup* Setup);

	void PreloadAllMaps(const TArray<USFCondition*> &Conditions);

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

	// this is used if the study should not be started from the beginning
	// e.g., when starting a map directly for debugging in the editor
	// or when continuing a crashed study run
	USFCondition* StartCondition;

	// State of Study / GameInstance
	UPROPERTY(BlueprintReadOnly)
	bool bStudyStarted = false;

	UPROPERTY(BlueprintReadOnly)
	bool bStudyEnded = false;

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

