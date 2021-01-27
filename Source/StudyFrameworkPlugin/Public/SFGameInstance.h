// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SFParticipant.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SFDefinesPublic.h"
#include "SFDefinesPrivate.h"
#include "SFStudyPhase.h"

#include "SFLogger.h"
#include "SFStudyControllerActor.h"



#include "SFGameInstance.generated.h"

class USFFadeHandler;

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFGameInstance : public UGameInstance
{
	GENERATED_BODY()

   
public: // TODO check what can be protected:
    UFUNCTION()
	    bool StartStudy();  
    UFUNCTION()
        void EndStudy();    // TODO implement EndStudy()
    UFUNCTION()
        bool NextSetup();
    UFUNCTION()
        bool IsStarted() const;

    // instead use:
    UFUNCTION()
        void AddPhase(USFStudyPhase* Phase);

    // Load for each level
    UFUNCTION()
        void AddActorForEveryLevelInEveryPhaseCpp(UClass* Actor);
    UFUNCTION()
        void AddActorForEveryLevelInEveryPhaseBlueprint(FSFClassOfBlueprintActor Actor);


    // Handling Study 
    UFUNCTION()
        void SpawnAllActorsForLevel();
    UFUNCTION()
        void SpawnBlueprintActor(FSFClassOfBlueprintActor Actor) const;
    UFUNCTION()
        void OnLevelLoaded();



    UFUNCTION()
        bool IsInitialized() const;
    UFUNCTION()
        void Initialize();

    UFUNCTION()
        bool HasData() const;               // TODO Was damit machen?

    // Fade Handler
    UFUNCTION()
        void SetFadeColor(FLinearColor Color);
    UFUNCTION()
        void SetFadeDuration(float FadeDuration);
    UFUNCTION()
        void SetFadedOutDuration(float FadeOutWait);
    UFUNCTION()
        void SetInitialFadedOut(bool bFadedOut);



    UFUNCTION()
        void SaveData(const FString Where, FString Data);

    UFUNCTION()
        void LogData(const FString String);

    UFUNCTION()
        void CommitData();


protected:

    UPROPERTY()
        TArray<UClass*> SpawnInEveryPhaseCpp;

    UPROPERTY()
        TArray<FSFClassOfBlueprintActor> SpawnInEveryPhaseBlueprint;


	UPROPERTY()
		bool bInitialized = false;

    UPROPERTY()
        bool bStudyStarted = false;

	UPROPERTY()
		USFFadeHandler* FadeHandler;

    UPROPERTY()
		USFParticipant* Participant;


    // Phase stuff
    UPROPERTY()
        TArray<USFStudyPhase*> Phases;

    UPROPERTY()
        USFStudyPhase* CurrentPhase;

    UPROPERTY()
        int CurrentPhaseIdx;

    UPROPERTY()
        USFLogger* Logger;

};
