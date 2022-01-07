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

	UFUNCTION(BlueprintCallable,  meta = (DisplayName="Get SFGameInstance"))
	static USFGameInstance* Get();
	static bool IsInitialized();

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

	UFUNCTION(BlueprintCallable)
	void LogData(const FString& DependenVariableName, const FString& Value);
	UFUNCTION(BlueprintCallable)
	void LogComment(const FString& Comment);

	UFUNCTION()
	void LogToHUD(FString Text);

	
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

