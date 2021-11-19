// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SFConditionListEntry.h"

#include "Help/SFUtils.h"
#include "SFGameInstance.h"


void USFConditionListEntry::FillWithCondition(const USFCondition* InCondition)
{
	Condition = InCondition;
	TArray<FString> Data;
	Data.Add(Condition->PhaseName);
	Data.Add(FPaths::GetBaseFilename(Condition->Map));
	Data.Add(Condition->TimeTaken <= 0.0 ? "-" : FString::FromInt(Condition->TimeTaken));
	for (auto Factor : Condition->FactorLevels)
	{
		Data.Add(Factor.Value);
	}
	TextBlockIdToDependentVar.Empty();
	for (auto DependentVar : Condition->DependentVariablesValues)
	{
		TextBlockIdToDependentVar.Add(Data.Num() - 3, DependentVar.Key);
		//-3 since the first three elements of Data or not mapped to the text fields
		Data.Add(DependentVar.Value == "" ? "-" : DependentVar.Value);
	}
	FillTextsHelper(Data);
	IsHeader = false;
}

void USFConditionListEntry::FillAsPhaseHeader(const USFCondition* InCondition)
{
	Condition = InCondition;
	TArray<FString> Data;
	Data.Add("Phase");
	Data.Add("Map");
	Data.Add("Duration");
	for (auto Factor : Condition->FactorLevels)
	{
		Data.Add(Factor.Key);
	}
	for (auto DependentVar : Condition->DependentVariablesValues)
	{
		Data.Add(DependentVar.Key->Name);
	}
	FillTextsHelper(Data);
	GoToButton->SetVisibility(ESlateVisibility::Hidden);
	IsHeader = true;
}

void USFConditionListEntry::FillTextsHelper(const TArray<FString>& Data)
{
	Phase->SetText(FText::FromString(Data[0]));
	Map->SetText(FText::FromString(Data[1]));
	Time->SetText(FText::FromString(Data[2]));

	int UsedTexts = 0;
	TArray<UTextBlock*> Texts = {Text0, Text1, Text2, Text3};

	for (int i = 3; i < Data.Num(); ++i)
	{
		Texts[UsedTexts++]->SetText(FText::FromString(Data[i]));
		if (UsedTexts >= Texts.Num())
		{
			FSFUtils::Log(
				"[USFConditionListEntry::FillWithCondition] to few text fields to show everything, " + FString::FromInt(
					Condition->FactorLevels.Num() + Condition->DependentVariablesValues.Num()) +
				"text fields would be needed.", true);
			return;
		}
	}

	//hide unused texts
	while (UsedTexts < Texts.Num())
	{
		Texts[UsedTexts++]->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USFConditionListEntry::GoTo()
{
	USFGameInstance::Get()->GoToCondition(Condition);
}

void USFConditionListEntry::UpdateData()
{
	IsActive = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition() == Condition;
	IsDone = Condition->bConditionFinished;

	TArray<UTextBlock*> Texts = {Text0, Text1, Text2, Text3, Time};
	for (int i = 0; i < Texts.Num(); ++i)
	{
		if (Texts[i]->Text.ToString() != "-")
		{
			continue;
		}
		FString NewValue = "";
		//if it is "-" check whether we have new data?
		if (i == Texts.Num() - 1) //Time
		{
			if (Condition->TimeTaken <= 0.0)
			{
				continue;
			}
			NewValue = FString::FromInt(Condition->TimeTaken);
		}
		else
		{
			FString Value = Condition->DependentVariablesValues[TextBlockIdToDependentVar[i]];
			if (Value == "")
			{
				continue;
			}
			NewValue = Value;
		}
		Texts[i]->SetText(FText::FromString(NewValue));
	}

	if (IsHeader)
	{
		BackgroundColor->SetBrushColor(HeaderColor);
	}
	else if (IsDone)
	{
		BackgroundColor->SetBrushColor(DoneColor);
	}
	else if (IsActive)
	{
		BackgroundColor->SetBrushColor(ActiveColor);
	}
	else
	{
		BackgroundColor->SetBrushColor(DefaultColor);
	}
}
