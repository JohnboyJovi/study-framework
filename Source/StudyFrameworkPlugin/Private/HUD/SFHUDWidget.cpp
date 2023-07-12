// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SFHUDWidget.h"

#include "Help/SFUtils.h"
#include "Logging/SFLoggingUtils.h"


USFHUDWidget::USFHUDWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USFHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USFHUDWidget::SetParticipant(const FString& Text)
{
	ParticipantTextBox->SetText(FText::FromString(FString(TEXT("Participant: ")) + Text));
}

void USFHUDWidget::SetPhase(const FString& Text)
{
	PhaseTextBox->SetText(FText::FromString(FString(TEXT("Phase: ")) + Text));
}

void USFHUDWidget::SetCondition(const TMap<FString, FString>& Texts)
{
	//TODO: as of now we have a fixed number of text blocks, maybe this can be made dynamic
	TArray<UTextBlock*> TextBlocks = {
		ConditionText1, ConditionText2, ConditionText3, ConditionText4, ConditionText5, ConditionText6
	};
	int TextIndex=0;
	for(auto FactorLevel : Texts)
	{
		if(TextIndex>=TextBlocks.Num())
		{
			FSFLoggingUtils::Log("Not all factor levels fit in the HUD, so only showing the first 6 levels");
			break;
		}
		TextBlocks[TextIndex++]->SetText(FText::FromString(FactorLevel.Key + ": "+FactorLevel.Value));
	}
	while(TextIndex<TextBlocks.Num())
	{
		TextBlocks[TextIndex++]->SetVisibility(ESlateVisibility::Collapsed);
	}
	ConditionTexts = Texts;
}

void USFHUDWidget::SetStatus(const FString& Text)
{
	StatusTextBox->SetText(FText::FromString(FString(TEXT("Status: ")) + Text));
}

void USFHUDWidget::AddLogMessage(const FString& Text)
{
	if (!Text.IsEmpty())
		LogMessages.Add(Text);

	while (LogMessages.Num() > MaxNumberShownLogMessages)
		LogMessages.RemoveAt(0);

	FString LogString = "";
	for (int i= LogMessages.Num()-1; i>=0; --i)
	{
		FString Message = LogMessages[i];
		LogString += Message + "\n";
	}
	LogsTextBox->SetText(FText::FromString(LogString));
}


void USFHUDWidget::ClearWidget()
{
	SetCondition({});
	SetParticipant("???");
	SetPhase("???");
	SetStatus("???");
}

FHUDSavedData USFHUDWidget::GetData()
{
	FHUDSavedData Data;
	Data.Status = StatusTextBox->GetText().ToString();
	Data.Participant = ParticipantTextBox->GetText().ToString();
	Data.Phase = PhaseTextBox->GetText().ToString();
	Data.Condition = ConditionTexts;
	Data.LogMessages = LogMessages;
	return Data;
}

void USFHUDWidget::SetData(FHUDSavedData Data)
{
	if (Data.Status.StartsWith("Status:"))
		StatusTextBox->SetText(FText::FromString(Data.Status));
	else
		SetStatus(Data.Status);

	if (Data.Participant.StartsWith("Participant:"))
		ParticipantTextBox->SetText(FText::FromString(Data.Participant));
	else
		SetParticipant(Data.Participant);

	if (Data.Phase.StartsWith("Phase:"))
		PhaseTextBox->SetText(FText::FromString(Data.Phase));
	else
		SetPhase(Data.Phase);

	SetCondition(Data.Condition);

	LogMessages = Data.LogMessages;
	AddLogMessage("");
}

void USFHUDWidget::SetCursorWidgetPosition(FVector2D Pos)
{
	CursorImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	CursorImage->SetRenderTranslation(Pos);

	//rather use CanvasPanelSlot::SetPosition(FVector2D InPosition)?
}

UButton* USFHUDWidget::GetStartButton()
{
	return StartButton;
}

UButton* USFHUDWidget::GetNextButton()
{
	return NextButton;
}

UButton* USFHUDWidget::GetShowConditionsButton()
{
	return ShowConditionList;
}

UScrollBox* USFHUDWidget::GetConditionList()
{
	return ConditionList;
}
