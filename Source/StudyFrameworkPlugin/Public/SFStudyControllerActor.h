// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SFDefinesPublic.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFStudyPhase.h"

#include "SFStudyControllerActor.generated.h"



class USFGameInstance;



UCLASS()
class STUDYFRAMEWORKPLUGIN_API ASFStudyControllerActor : public AActor // TODO rename StudyManager?
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	ASFStudyControllerActor();

	virtual void Tick(float DeltaTime) override;

	// TODO implement Start Study()
	UFUNCTION(BlueprintCallable)
	    bool StartStudy();

	UFUNCTION(BlueprintCallable)
	    bool NextSetup();

    UFUNCTION()
        bool AddPhase(USFStudyPhase* Phase);


    // Each Level Load
    UFUNCTION()
        bool AddActorForEachLevelCpp(UClass* Actor);
    UFUNCTION()
        bool AddActorForEachLevelBlueprint(FSFClassOfBlueprintActor Actor);

    // Fading
    UFUNCTION()
        void SetFadeColor(FLinearColor Color);
    UFUNCTION()
        void SetFadeDuration(float FadeDuration);
    UFUNCTION()
        void SetFadedOutDuration(float FadeOutWait);
    UFUNCTION()
        bool SetInitialFadedOut(bool bFadedOut);


    UFUNCTION() 
        void SaveData(const FString DataName, FString Data);

    UFUNCTION()
        void LogData(const FString String);

    UFUNCTION()
        void CommitData();

    UFUNCTION()
        void Initialize();

    // TODO implement GetCurrentControllerActor to work non static
    UFUNCTION()
        static ASFStudyControllerActor* GetCurrentControllerActor();
	
protected:

    // TODO need CheckGameInstance()?
    UFUNCTION()
        bool CheckGameInstance();


    static ASFStudyControllerActor* Manager;

	UPROPERTY()
	    USFGameInstance* GameInstance;

    UPROPERTY()
        bool bInitialized = false;
};


