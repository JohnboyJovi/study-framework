// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SFMasterHUD.h"
#include "HUD/SFConditionListEntry.h"
#include "SFGameInstance.h"
#include "SFParticipant.h"
#include "SFPlugin.h"
#include "Help/SFUtils.h"


ASFMasterHUD::ASFMasterHUD()
{
}

void ASFMasterHUD::DrawHUD()
{
	DrawBackground();

	if (FSFPlugin::GetIsMaster())
		Super::DrawHUD();
}

void ASFMasterHUD::BeginPlay()
{
	//is called also every time the map is changed (a new condition is loaded)
	Super::BeginPlay();
	if (SFWidgetClass)
	{
		HUDWidget = CreateWidget<USFHUDWidget>(GetWorld(), SFWidgetClass);

		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	FHUDSavedData& Data = USFGameInstance::Get()->HUDSavedData;
	if (Data.bSet && HUDWidget)
	{
		HUDWidget->SetData(Data);
	}
	else
	{
		HUDWidget->SetStatus("Wait for start");
	}

	if (USFGameInstance::Get()->IsStarted())
	{
		HUDWidget->GetStartButton()->SetVisibility(ESlateVisibility::Collapsed);
	}

	HUDWidget->GetConditionList()->SetVisibility(ESlateVisibility::Collapsed);

	HUDWidget->GetStartButton()->OnClicked.AddDynamic(this, &ASFMasterHUD::OnStartButtonPressed);
	HUDWidget->GetNextButton()->OnClicked.AddDynamic(this, &ASFMasterHUD::OnNextButtonPressed);
	HUDWidget->GetShowConditionsButton()->OnClicked.AddDynamic(this, &ASFMasterHUD::OnShowConditionsButtonPressed);
}

void ASFMasterHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!HUDWidget)
		return;
	Cast<USFGameInstance>(GetGameInstance())->HUDSavedData = HUDWidget->GetData();
}

void ASFMasterHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASFMasterHUD::UpdateHUD(USFParticipant* Participant, FString Status)
{
	if (HUDWidget == nullptr)
		return;

	HUDWidget->SetStatus(Status);

	if (Participant == nullptr)
		return;

	HUDWidget->SetParticipant(Participant->GetID());

	const USFCondition* Condition = Participant->GetCurrentCondition();
	HUDWidget->SetPhase(Condition->PhaseName);

	FString ConditionString = "(";
	ConditionString += "Map: " + FPaths::GetBaseFilename(Condition->Map);
	for (auto FactorLevel : Condition->FactorLevels)
	{
		ConditionString += "; " + FactorLevel.Key + ": " + FactorLevel.Value;
	}
	ConditionString += ")";
	HUDWidget->SetCondition(ConditionString);
}

void ASFMasterHUD::AddLogMessage(FString Text)
{
	HUDWidget->AddLogMessage(Text);
}


void ASFMasterHUD::SetBackgroundColor(FLinearColor Color)
{
	BackgroundColor = Color;
}

void ASFMasterHUD::DrawBackground()
{
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	DrawRect(BackgroundColor, 0.0f, 0.0f, ViewportSize.X, ViewportSize.Y);
}

void ASFMasterHUD::OnStartButtonPressed()
{
	HUDWidget->GetStartButton()->SetVisibility(ESlateVisibility::Collapsed);
	USFGameInstance::Get()->StartStudy();
}

void ASFMasterHUD::OnNextButtonPressed()
{
	USFGameInstance::Get()->NextCondition();
}

void ASFMasterHUD::OnShowConditionsButtonPressed()
{
	if (bShowConditionList)
	{
		HUDWidget->GetConditionList()->SetVisibility(ESlateVisibility::Collapsed);
		Cast<UTextBlock>(HUDWidget->GetShowConditionsButton()->GetAllChildren()[0])->SetText(FText::FromString("Show Conditions"));
		bShowConditionList = false;
	}
	else
	{
		bShowConditionList = true;
		Cast<UTextBlock>(HUDWidget->GetShowConditionsButton()->GetAllChildren()[0])->SetText(FText::FromString("Hide Conditions"));
		UScrollBox* ConditionList = HUDWidget->GetConditionList();
		ConditionList->ClearChildren();
		ConditionList->SetVisibility(ESlateVisibility::Visible);

		FString LastPhase = "";

		const TArray<USFCondition*> Conditions = USFGameInstance::Get()->GetParticipant()->GetAllConditions();

		for (const USFCondition* Condition : Conditions)
		{
			if(LastPhase != Condition->PhaseName)
			{
				//add phase header first
				USFConditionListEntry* Entry = CreateWidget<USFConditionListEntry>(
				GetWorld()->GetFirstPlayerController(), SFConditionListEntryBP_Class);
				Entry->FillAsPhaseHeader(Condition);
				LastPhase = Condition->PhaseName;
				ConditionList->AddChild(Entry);
			}
			USFConditionListEntry* Entry = CreateWidget<USFConditionListEntry>(
				GetWorld()->GetFirstPlayerController(), SFConditionListEntryBP_Class);
			Entry->FillWithCondition(Condition);
			ConditionList->AddChild(Entry);
		}
	}
}
