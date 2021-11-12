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
	FString Condition;
	TArray<FString> LogMessages;
	bool bSet = false;
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


	void SetParticipant(FString Text);
	void SetPhase(FString Text);
	void SetCondition(FString Text);
	void SetStatus(FString Text);

	void AddLogMessage(FString Text);

	void ClearWidget();

	FHUDSavedData GetData();
	void SetData(FHUDSavedData Data);

	UButton* GetStartButton();
	UButton* GetNextButton();
	UButton* GetShowConditionsButton();

private:

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* StatusTextBox;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* ParticipantTextBox;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* PhaseTextBox;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* ConditionTextBox;

	TArray<FString> LogMessages;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* LogsTextBox;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* StartButton;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* NextButton;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* ShowConditionList;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UScrollBox* ConditionList;
};
