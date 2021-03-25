// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SFParticipant.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SFDefinesPublic.h"
#include "SFDefinesPrivate.h"
#include "SFStudyPhase.h"
#include "SFMasterHUD.h"

#include "SFLogger.h"
#include "SFStudyControllerActor.h"



#include "SFGameInstance.generated.h"

class USFFadeHandler;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFadedInDelegate);

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFGameInstance : public UGameInstance
{
	GENERATED_BODY()

   
public: // TODO check what can be protected:

    // ****************************************************************** // 
    // ******* Initialization ******************************************* //
    // ****************************************************************** //
    UFUNCTION()
        void Initialize(FString ParticipantID, FString JsonFilePath);
    UFUNCTION()
        bool IsInitialized() const;

    // ****************************************************************** // 
    // ******* Control Study ******************************************** //
    // ****************************************************************** //
    UFUNCTION()
	    bool StartStudy();  
    UFUNCTION()
        void EndStudy();    // TODO implement EndStudy()
    UFUNCTION()
        bool NextCondition();
    UFUNCTION()
        bool IsStarted() const;


    UFUNCTION()
        void SaveData(const FString Where, FString Data);
    UFUNCTION()
        void SaveDataArray(const FString Where, TArray<FString> Data);
    UFUNCTION()
        void CommitData();

    UFUNCTION()
        void LogData(const FString String);

	UFUNCTION()
		void LogToHUD(FString Text);

    // ****************************************************************** // 
    // ******* Prepare Study ******************************************** //
    // ****************************************************************** //
    UFUNCTION()
        void AddPhase(USFStudyPhase* Phase);

    UFUNCTION()
        void AddActorForEveryLevelInEveryPhaseCpp(UClass* Actor);
    UFUNCTION()
        void AddActorForEveryLevelInEveryPhaseBlueprint(FSFClassOfBlueprintActor Actor);

    // Fade Handler
    UFUNCTION()
        void SetFadeColor(FLinearColor Color);
    UFUNCTION()
        void SetFadeDuration(float FadeDuration);
    UFUNCTION()
        void SetFadedOutDuration(float FadeOutWait);
    UFUNCTION()
        void SetInitialFadedOut(bool bFadedOut);


    // ****************************************************************** // 
    // ******* Executing Study ****************************************** //
    // ****************************************************************** //
    UFUNCTION()
        void SpawnAllActorsForLevel();
    UFUNCTION()
        void SpawnBlueprintActor(FSFClassOfBlueprintActor Actor) const;
    UFUNCTION()
        void OnLevelLoaded();
	UFUNCTION()
		  void OnFadedIn();

	 // ****************************************************************** // 
    // *******   Delegates   ******************************************** //
    // ****************************************************************** //
	
	UPROPERTY(BlueprintAssignable)
	 FOnFadedInDelegate OnFadedInDelegate;


	

    UFUNCTION()
        void UpdateHUD(FString Status);


	//this is used by the SFMasterHUD to store content between levels
	FHUDSavedData HUDSavedData;

protected:
    // Handlich Stuff
    UPROPERTY()
	USFParticipant* Participant;
    UPROPERTY()
	USFFadeHandler* FadeHandler;

    // Spawn in every Level
    UPROPERTY()
        TArray<UClass*> SpawnInEveryPhaseCpp;
    UPROPERTY()
        TArray<FSFClassOfBlueprintActor> SpawnInEveryPhaseBlueprint;

    // State of Study / GameInstance
    UPROPERTY()
	 bool bInitialized = false;
    UPROPERTY()
    bool bStudyStarted = false;

};
