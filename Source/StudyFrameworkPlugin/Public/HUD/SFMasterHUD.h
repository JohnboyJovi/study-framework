// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "Components/WidgetComponent.h"

#include "HUD/SFHUDWidget.h"
#include "HUD/SFConditionListEntry.h"
#include "HUD/SFHMDSpectatorHUDHelp.h"

#include "SFMasterHUD.generated.h"

class USFParticipant;
/**
 * 
 */
UCLASS()
class STUDYFRAMEWORKPLUGIN_API ASFMasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASFMasterHUD();

	virtual void DrawHUD() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;

	bool IsWidgetPresent() const;

	UFUNCTION()
	void UpdateHUD(USFParticipant* Participant, const FString& Status = "None");

	UFUNCTION()
	void AddLogMessage(const FString& Text);

	UFUNCTION()
	void SetBackgroundAlpha(float Alpha);

	UFUNCTION()
	void SetStartStudyButtonVisibility(ESlateVisibility Visibility);
	UFUNCTION()
	void SetNextConditionButtonVisibility(ESlateVisibility Visibility);

	UPROPERTY(EditDefaultsOnly, Category = "Interactive")
	TSubclassOf<USFHUDWidget> SFWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "Interactive")
	TSubclassOf<USFConditionListEntry> SFConditionListEntryBP_Class;

	UFUNCTION()
	void OnStartButtonPressed();
	UFUNCTION()
	void OnNextButtonPressed();
	UFUNCTION()
	void OnShowConditionsButtonPressed();

	const ASFHMDSpectatorHUDHelp* GetHUDHelper();

	//setting HUD in-/visibile at runtime
	void SetHUDVisible(bool bVisible);

protected:
	UPROPERTY(VisibleAnywhere)
	USFHUDWidget* HUDWidget;


private:
	void DrawBackground();
	float BackgroundAlpha = 1.0f;

	ASFHMDSpectatorHUDHelp* HMDHUDHelper;
	bool bHMDHUDHelperTextureSet= false;

	bool bShowConditionList = false;

	USFParticipant* UnForwardedParticipant = nullptr;
	FString UnforwardedStatus;
};
