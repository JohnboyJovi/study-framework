// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "Components/WidgetComponent.h"

#include "SFHUDWidget.h"

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

	 UFUNCTION()
		 void UpdateHUD(USFParticipant* Participant, FString Status = "None");

	UFUNCTION()
		void AddLogMessage(FString Text);

	 UFUNCTION()
		 void SetBackgroundColor(FLinearColor Color);

    UPROPERTY(EditDefaultsOnly, Category = "Interactive")
        TSubclassOf<USFHUDWidget> SFWidgetClass;

	UFUNCTION()
	void OnStartButtonPressed();
	UFUNCTION()
	void OnNextButtonPressed();

private:
    UPROPERTY()
        USFHUDWidget* HUDWidget;
	 UPROPERTY()
		 FLinearColor BackgroundColor = FLinearColor::Black;
	 void DrawBackground();
	
};
