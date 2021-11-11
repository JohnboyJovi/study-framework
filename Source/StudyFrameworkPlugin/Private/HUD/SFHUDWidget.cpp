// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SFHUDWidget.h"

#include "Help/SFUtils.h"


USFHUDWidget::USFHUDWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USFHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USFHUDWidget::SetParticipant(FString Text)
{
	ParticipantTextBox->SetText(FText::FromString(FString(TEXT("Participant: ")) + Text));
}

void USFHUDWidget::SetPhase(FString Text)
{
	PhaseTextBox->SetText(FText::FromString(FString(TEXT("Phase: ")) + Text));
}

void USFHUDWidget::SetCondition(FString Text)
{
	ConditionTextBox->SetText(FText::FromString(FString(TEXT("Condition: ")) + Text));
}

void USFHUDWidget::SetStatus(FString Text)
{
	StatusTextBox->SetText(FText::FromString(FString(TEXT("Status: ")) + Text));
}

void USFHUDWidget::AddLogMessage(FString Text)
{
	if (!Text.IsEmpty())
		LogMessages.Add(Text);

	while (LogMessages.Num() > 5)
		LogMessages.RemoveAt(0);

	FString LogString = "";
	for (FString Message : LogMessages)
	{
		LogString += Message + "\n";
	}
	LogsTextBox->SetText(FText::FromString(LogString));
}


void USFHUDWidget::ClearWidget()
{
	SetCondition("???");
	SetParticipant("???");
	SetPhase("???");
	SetStatus("???");
}

FHUDSavedData USFHUDWidget::GetData()
{
	FHUDSavedData Data;
	Data.bSet = true;
	Data.Status = StatusTextBox->GetText().ToString();
	Data.Participant = ParticipantTextBox->GetText().ToString();
	Data.Phase = PhaseTextBox->GetText().ToString();
	Data.Condition = ConditionTextBox->GetText().ToString();
	Data.LogMessages = LogMessages;
	return Data;
}

void USFHUDWidget::SetData(FHUDSavedData Data)
{
	StatusTextBox->SetText(FText::FromString(Data.Status));
	ParticipantTextBox->SetText(FText::FromString(Data.Participant));
	PhaseTextBox->SetText(FText::FromString(Data.Phase));
	ConditionTextBox->SetText(FText::FromString(Data.Condition));
	LogMessages = Data.LogMessages;
	AddLogMessage("");
}

UButton* USFHUDWidget::GetStartButton()
{
	return StartButton;
}

UButton* USFHUDWidget::GetNextButton()
{
	return NextButton;
}
