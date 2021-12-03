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


public: // TODO check what can be protected:

	//override UGameInstance methods
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;

	static USFGameInstance* Get();
	static bool IsInitialized();

	// ****************************************************************** // 
	// ******* Initialization ******************************************* //
	// ****************************************************************** //
	UFUNCTION()
	void PrepareForParticipant(int ParticipantID, FString JsonFilePath = "WhatFor");
	UFUNCTION()
	bool IsPrepared() const;
	

	// ****************************************************************** // 
	// ******* Control Study ******************************************** //
	// ****************************************************************** //
	UFUNCTION(BlueprintCallable)
	bool StartStudy(const USFCondition* StartCondition = nullptr);
	UFUNCTION(BlueprintCallable)
	void EndStudy(); // TODO implement EndStudy()
	UFUNCTION(BlueprintCallable)
	bool NextCondition();
	UFUNCTION(BlueprintCallable)
	bool GoToCondition(const USFCondition* Condition);
	UFUNCTION(BlueprintCallable)
	bool IsStarted() const;

	UFUNCTION(BlueprintCallable)
	FString GetFactorLevel(FString FactorName) const;


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
	USFStudySetup* CreateNewStudySetup();
	UFUNCTION()
	USFStudySetup* GetStudySetup();

	//TODO: create this!
	UFUNCTION()
	void LoadStudySetupFromJson();
	UFUNCTION()
	void SaveStudySetupToJson(FString Filename) const;

	// Fade Handler
	UFUNCTION()
	USFFadeHandler* GetFadeHandler();

	// ****************************************************************** // 
	// ******* Executing Study ****************************************** //
	// ****************************************************************** //
	// 
	UFUNCTION()
	void SpawnAllActorsForLevel();
	UFUNCTION()
	void OnLevelLoaded();
	UFUNCTION()
	void OnFadedIn();
	UFUNCTION()
	USFParticipant* GetParticipant() const;

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

	void GoToConditionSynced(FString ConditionName);
	DECLARE_DISPLAY_CLUSTER_EVENT(USFGameInstance, GoToConditionSynced);

	void RestoreLastParticipant(USFCondition* StartCondition);
	//method called by a timer if we want to directly fade in on startup
	void StartFadingIn();
	FTimerHandle StartFadingTimerHandle;

	UPROPERTY()
	USFStudySetup* StudySetup = nullptr;
	UPROPERTY()
	USFParticipant* Participant = nullptr;


	UPROPERTY()
	USFFadeHandler* FadeHandler;

	// State of Study / GameInstance
	UPROPERTY()
	bool bPrepared = false;
	UPROPERTY()
	bool bStudyStarted = false;

	//singleton object of this class, to easier access it
	static USFGameInstance* Instance;

	// this is only needed if a map should be started for debugging, but Init wasn't called
	USFCondition* ConditionToStartAtInit=nullptr;
};

