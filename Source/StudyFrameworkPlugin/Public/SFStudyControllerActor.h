// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SFDefinesPublic.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFStudyPhase.h"


#include "SFStudyControllerActor.generated.h"


class ASFPlayerController;
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


	// ****************************************************************** // 
	// ******* Control Study ******************************************** //
	// ****************************************************************** //

	UFUNCTION(BlueprintCallable)
	bool StartStudy(); // TODO implement Start Study()

	UFUNCTION(BlueprintCallable)
	bool NextCondition();


	UFUNCTION(BlueprintCallable) // TODO Kommentar
	void SaveDataArray(const FString DataName, TArray<FString> Data);

	UFUNCTION(BlueprintCallable) // TODO Kommentar
	void SaveData(const FString DataName, FString Data); // TODO ??

	UFUNCTION(BlueprintCallable) // TODO Kommentar
	void LogData(const FString String); // TODO Difference here?


	// ****************************************************************** // 
	// ******* Prepare Study ******************************************** //
	// ****************************************************************** //

	UFUNCTION()
	void Initialize(FString ParticipantID, FString JsonFilePath);


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


	// TODO implement GetCurrentControllerActor to work non static
	UFUNCTION()
	static ASFStudyControllerActor* GetCurrentControllerActor();

protected:

	static ASFStudyControllerActor* Manager;

	UPROPERTY()
	USFGameInstance* GameInstance;

	UPROPERTY()
	bool bInitialized = false;
};
