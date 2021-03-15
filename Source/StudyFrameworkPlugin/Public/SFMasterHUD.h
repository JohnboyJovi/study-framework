// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "Components/WidgetComponent.h"

#include "SFHUDWidget.h"

#include "SFMasterHUD.generated.h"

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

    virtual void Tick(float DeltaSeconds) override;

    // UPROPERTY()
    void SetJsonData(TSharedPtr<FJsonObject> Data);

    UFUNCTION()
        void SetText(FString Text);

    UFUNCTION()
        void ClearWidget();

	 UFUNCTION()
		 void SetBackgroundColor(FLinearColor Color);

    UPROPERTY(EditDefaultsOnly, Category = "Interactive")
        TSubclassOf<USFHUDWidget> SFWidgetClass;

private:
    UPROPERTY()
        USFHUDWidget* SFWidget;
	 UPROPERTY()
		 FLinearColor BackgroundColor = FLinearColor::Black;
	 void DrawBackground();
	
};
