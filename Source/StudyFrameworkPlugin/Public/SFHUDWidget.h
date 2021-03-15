// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"

// #include "JsonObject.h"
// #include "SharedPointer.h"


#include "SFHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:

	USFHUDWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    
    void SetJsonData(TSharedPtr<FJsonObject> Data);
    void SetText(FString Text);

    void ClearWidget();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* StatusTextBox;
	
	 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		 class UTextBlock* ParticipantTextBox;
	 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		 class UTextBlock* PhaseTextBox;
	 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		 class UTextBlock* ConditionTextBox;
	
	 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		 class UButton* StartButton;
	 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		 class UButton* NextButton;
};
