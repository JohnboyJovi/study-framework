// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SFMasterHUD.h"
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
	Super::BeginPlay();
	if (SFWidgetClass)
	{
		HUDWidget = CreateWidget<USFHUDWidget>(GetWorld(), SFWidgetClass);

		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	FHUDSavedData& Data = Cast<USFGameInstance>(GetGameInstance())->HUDSavedData;
	if (Data.bSet && HUDWidget)
	{
		HUDWidget->SetData(Data);
	}

	HUDWidget->GetStartButton()->OnClicked.AddDynamic(this, &ASFMasterHUD::OnStartButtonPressed);
	HUDWidget->GetNextButton()->OnClicked.AddDynamic(this, &ASFMasterHUD::OnNextButtonPressed);

	HUDWidget->SetStatus("Wait for start");
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

	USFStudyPhase* Phase = Participant->GetCurrentPhase();
	HUDWidget->SetPhase(Phase->GetName());


	USFCondition* Condition = Phase->GetCurrentCondition();

	FString ConditionString = "(";
	ConditionString += FPaths::GetBaseFilename(Condition->Map);
	for (auto FactorLevel : Condition->FactorLevels)
	{
		ConditionString += FactorLevel.Key + ": " + FactorLevel.Value + " ";
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
	USFGameInstance::Get()->StartStudy();
}

void ASFMasterHUD::OnNextButtonPressed()
{
	USFGameInstance::Get()->NextCondition();
}
