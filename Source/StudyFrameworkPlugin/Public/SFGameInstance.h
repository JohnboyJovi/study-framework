// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SFParticipant.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SFStudySetup.h"
#include "HUD/SFMasterHUD.h"

#include "SFGameInstance.generated.h"

class USFFadeHandler;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFadedInDelegate);

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFGameInstance : public UGameInstance
{
	GENERATED_BODY()


public: // TODO check what can be protected:

	//override UGameInstance init
	void Init() override;

	static USFGameInstance* Get();

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
	UFUNCTION(BlueprintCallable)
	bool StartStudy();
	UFUNCTION(BlueprintCallable)
	void EndStudy(); // TODO implement EndStudy()
	UFUNCTION(BlueprintCallable)
	bool NextCondition();
	UFUNCTION(BlueprintCallable)
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
	// 
	UFUNCTION()
	void SpawnAllActorsForLevel();
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

	UPROPERTY()
	USFStudySetup* StudySetup = nullptr;
	UPROPERTY()
	USFParticipant* Participant = nullptr;


	UPROPERTY()
	USFFadeHandler* FadeHandler;

	// State of Study / GameInstance
	UPROPERTY()
	bool bInitialized = false;
	UPROPERTY()
	bool bStudyStarted = false;

	//singleton object of this class, to easier access it
	static USFGameInstance* Instance;
};
