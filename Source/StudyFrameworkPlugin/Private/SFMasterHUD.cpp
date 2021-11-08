// Fill out your copyright notice in the Description page of Project Settings.


#include "SFMasterHUD.h"
#include "SFGameInstance.h"
#include "SFParticipant.h"
#include "SFPlugin.h"
#include "SFUtils.h"


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


	TArray<int> Condition = Phase->GetCurrentCondition();
	if (Condition.Num() == 0)
		return;

	FString ConditionString = "(";
	const TArray<FString>& Maps = Phase->GetMapNames();
	const int MapIndex = Condition[0];
	ConditionString += FPaths::GetBaseFilename(Maps[MapIndex]);
	const TArray<FSFStudyFactor>& Factors = Phase->GetFactors();
	for (int FactorIndex = 0; FactorIndex < Factors.Num(); ++FactorIndex)
	{
		const FSFStudyFactor Factor = Factors[FactorIndex];
		ConditionString += Factor.Name + ": " + FString::FromInt(Condition[FactorIndex + 1]) + " ";
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
