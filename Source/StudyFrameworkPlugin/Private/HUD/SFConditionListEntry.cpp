// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SFConditionListEntry.h"

#include "SFGameInstance.h"
#include "Logging/SFLoggingUtils.h"


void USFConditionListEntry::FillWithCondition(const USFCondition* InCondition)
{
	Condition = InCondition;
	TArray<FString> Data;
	Data.Add(Condition->PhaseName);
	Data.Add(Condition->GetTimeTaken() <= 0.0 ? "-" : FString::Printf(TEXT("%.2f"), Condition->GetTimeTaken()));
	for (auto Factor : Condition->FactorLevels)
	{
		Data.Add(Factor.Value);
	}
	TextBlockIdToDependentVar.Empty();
	for (auto DependentVar : Condition->DependentVariables)
	{
		TextBlockIdToDependentVar.Add(Data.Num() - 2, DependentVar);
		//-2 since the first two elements of Data or not mapped to the text fields
		if(auto MultiTrialVar = Cast<USFMultipleTrialDependentVariable>(DependentVar))
		{
			Data.Add("# Trials: " + FString::FromInt(MultiTrialVar->Values.size()));
		}
		else {
			Data.Add(DependentVar->Value == "" ? "-" : DependentVar->Value);
		}
	}
	FillTextsHelper(Data);
	IsHeader = false;
}

void USFConditionListEntry::FillAsPhaseHeader(const USFCondition* InCondition)
{
	Condition = InCondition;
	TArray<FString> Data;
	Data.Add("Phase");
	Data.Add("Duration");
	for (auto Factor : Condition->FactorLevels)
	{
		Data.Add(Factor.Key);
	}
	for (auto DependentVar : Condition->DependentVariables)
	{
		Data.Add(DependentVar->Name);
	}
	FillTextsHelper(Data);
	GoToButton->SetVisibility(ESlateVisibility::Hidden);
	IsHeader = true;
}

void USFConditionListEntry::FillTextsHelper(const TArray<FString>& Data)
{
	Phase->SetText(FText::FromString(Data[0]));
	Time->SetText(FText::FromString(Data[1]));

	EceteraText->SetVisibility(ESlateVisibility::Collapsed);

	int UsedTexts = 0;
	TArray<UTextBlock*> Texts = {Text0, Text1, Text2, Text3, Text4, Text5, Text6, Text7};

	for (int i = 2; i < Data.Num(); ++i)
	{
		if (UsedTexts >= Texts.Num())
		{
			FString MissingTexts = "{ ";
			for(int j=i; j < Data.Num(); ++j)
			{
				MissingTexts += Data[j] + " ";
			}
			MissingTexts += "}";
			FSFLoggingUtils::Log(
				"[USFConditionListEntry::FillTextsHelper] to few text fields to show everything, " + FString::FromInt(
					Condition->FactorLevels.Num() + Condition->DependentVariables.Num()) +
				" text fields would be needed. Not showing: " + MissingTexts, false);
			EceteraText->SetVisibility(ESlateVisibility::Visible);
			return;
		}
		Texts[UsedTexts++]->SetText(FText::FromString(Data[i]));
	}

	//hide unused texts
	while (UsedTexts < Texts.Num())
	{
		Texts[UsedTexts++]->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USFConditionListEntry::GoTo()
{
	USFGameInstance::Get()->GoToCondition(Condition, true); //force the change
}

void USFConditionListEntry::UpdateData()
{
	IsActive = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition() == Condition && USFGameInstance::Get()->
		GetParticipant()->GetCurrentCondition()->WasStarted();
	IsDone = Condition->IsFinished();

	if (IsHeader)
	{
		BackgroundColor->SetBrushColor(HeaderColor);
		return; //no need to update anything!
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

	TArray<UTextBlock*> Texts = {Text0, Text1, Text2, Text3, Text4, Text5, Text6, Text7, Time};
	for (int i = 0; i < Texts.Num(); ++i)
	{
		FString NewValue = "";
		//if it is "-" check whether we have new data?
		if (i == Texts.Num() - 1) //Time
		{
			if (Condition->GetTimeTaken() <= 0.0)
			{
				continue;
			}
			NewValue = FString::Printf(TEXT("%.2f"), Condition->GetTimeTaken());
		}
		else
		{
			if(!TextBlockIdToDependentVar.Contains(i))
			{
				continue;
			}

			USFDependentVariable* DependentVar = TextBlockIdToDependentVar[i];
			if (auto MultiTrialVar = Cast<USFMultipleTrialDependentVariable>(DependentVar))
			{
				NewValue = "# Trials: " + FString::FromInt(MultiTrialVar->Values.size());
			}
			else {
				FString Value = TextBlockIdToDependentVar[i]->Value;
				if (Value == "")
				{
					continue;
				}
				NewValue = Value;
			}
		}
		Texts[i]->SetText(FText::FromString(NewValue));
	}
}
