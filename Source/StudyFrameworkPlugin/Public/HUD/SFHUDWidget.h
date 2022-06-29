// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"

// #include "JsonObject.h"
// #include "SharedPointer.h"


#include "SFHUDWidget.generated.h"

USTRUCT()
struct FHUDSavedData
{
	GENERATED_BODY()
	FString Status;
	FString Participant;
	FString Phase;
	TMap<FString, FString> Condition;
	TArray<FString> LogMessages;
};

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


	void SetParticipant(const FString& Text);
	void SetPhase(const FString& Text);
	void SetCondition(const TMap<FString, FString>& Texts);
	void SetStatus(const FString& Text);

	void AddLogMessage(const FString& Text);

	void ClearWidget();

	FHUDSavedData GetData();
	void SetData(FHUDSavedData Data);

	void SetCursorWidgetPosition(FVector2D Pos);

	UButton* GetStartButton();
	UButton* GetNextButton();
	UButton* GetShowConditionsButton();
	UScrollBox* GetConditionList();

private:

	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* StatusTextBox;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* ParticipantTextBox;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* PhaseTextBox;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* ConditionText1;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* ConditionText2;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* ConditionText3;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* ConditionText4;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* ConditionText5;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* ConditionText6;
	TMap<FString, FString> ConditionTexts;

	TArray<FString> LogMessages;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UTextBlock* LogsTextBox;

	UPROPERTY(EditAnywhere, meta = (BindWidget)) UButton* StartButton;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UButton* NextButton;
	UPROPERTY(EditAnywhere, meta = (BindWidget)) UButton* ShowConditionList;

	UPROPERTY(EditAnywhere, meta = (BindWidget)) UScrollBox* ConditionList;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UImage* CursorImage; //only used for HMD fake HUD

	UPROPERTY(EditAnywhere) int MaxNumberShownLogMessages = 7;
};
