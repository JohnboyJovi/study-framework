// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SFMasterHUD.h"
#include "HUD/SFConditionListEntry.h"
#include "SFGameInstance.h"
#include "SFParticipant.h"
#include "SFPlugin.h"
#include "Help/SFUtils.h"

#include "Utility/VirtualRealityUtilities.h"
#include "HeadMountedDisplayFunctionLibrary.h"


ASFMasterHUD::ASFMasterHUD()
{
}

void ASFMasterHUD::DrawHUD()
{
	if (UVirtualRealityUtilities::IsMaster())
	{
		DrawBackground();
	}

	if (HMDHUDHelper && !bHMDHUDHelperTextureSet && HMDHUDHelper->GetWidgetComponent()->GetRenderTarget() != nullptr)
	{
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenTexture(
			HMDHUDHelper->GetWidgetComponent()->GetRenderTarget());
		bHMDHUDHelperTextureSet = true;
	}

	Super::DrawHUD();
}

void ASFMasterHUD::BeginPlay()
{
	//is called also every time the map is changed (a new condition is loaded)
	Super::BeginPlay();

	if(!USFGameInstance::Get()->GetStudySetup()->FadeConfig.bShowHUD)
	{
		return;
	}

	if (SFWidgetClass)
	{
		HMDHUDHelper = nullptr;
		if (UVirtualRealityUtilities::IsHeadMountedMode())
		{
			HMDHUDHelper = Cast<ASFHMDSpectatorHUDHelp>(
				GetWorld()->SpawnActor(ASFHMDSpectatorHUDHelp::StaticClass()));
			HUDWidget = Cast<USFHUDWidget>(HMDHUDHelper->CreateWidget(SFWidgetClass));
		}
		else if (UVirtualRealityUtilities::IsMaster())
		{
			HUDWidget = CreateWidget<USFHUDWidget>(GetWorld(), SFWidgetClass);
		}

		if (HUDWidget)
		{
			if (HMDHUDHelper)
			{
				UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::TexturePlusEye);
				UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenModeTexturePlusEyeLayout(
					FVector2D(0, 0), FVector2D(1, 1), FVector2D(0, 0), FVector2D(1, 1), true, false, true);
			}
			else
			{
				HUDWidget->AddToViewport();
			}
		}
	}

	if (!HUDWidget)
		return;

	FHUDSavedData& Data = USFGameInstance::Get()->HUDSavedData;

	HUDWidget->SetData(Data);


	if (USFGameInstance::Get()->IsStarted())
	{
		HUDWidget->GetStartButton()->SetVisibility(ESlateVisibility::Collapsed);
	}

	if(!USFGameInstance::Get()->GetParticipant()->GetNextCondition())
	{
		Cast<UTextBlock>(HUDWidget->GetNextButton()->GetAllChildren()[0])->SetText(
			FText::FromString("End Study"));
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

bool ASFMasterHUD::IsWidgetPresent() const
{
	return HUDWidget != nullptr;
}

void ASFMasterHUD::UpdateHUD(USFParticipant* Participant, const FString& Status)
{
	if (HUDWidget == nullptr)
		return;

	HUDWidget->SetStatus(Status);

	if (Participant == nullptr){
		USFGameInstance::Get()->HUDSavedData = HUDWidget->GetData();
		return;
	}

	HUDWidget->SetParticipant(FString::FromInt(Participant->GetID()));

	USFCondition* Condition = Participant->GetCurrentCondition();

	if(!Condition){
		USFGameInstance::Get()->HUDSavedData = HUDWidget->GetData();
		return;
	}

	HUDWidget->SetPhase(Condition->PhaseName);
	HUDWidget->SetCondition(Condition->GetPrettyName());

	USFGameInstance::Get()->HUDSavedData = HUDWidget->GetData();
}

void ASFMasterHUD::AddLogMessage(const FString& Text)
{
	HUDWidget->AddLogMessage(Text);
}


void ASFMasterHUD::SetBackgroundColor(FLinearColor Color)
{
	BackgroundColor = Color;
}

void ASFMasterHUD::SetStartStudyButtonVisibility(ESlateVisibility Visibility)
{
	if(!HUDWidget)
		return;
	HUDWidget->GetStartButton()->SetVisibility(Visibility);
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
	if(!USFGameInstance::Get()->GetParticipant()->GetNextCondition())
	{
		HUDWidget->GetNextButton()->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ASFMasterHUD::OnShowConditionsButtonPressed()
{
	if (bShowConditionList)
	{
		HUDWidget->GetConditionList()->SetVisibility(ESlateVisibility::Collapsed);
		Cast<UTextBlock>(HUDWidget->GetShowConditionsButton()->GetAllChildren()[0])->SetText(
			FText::FromString("Show Conditions"));
		bShowConditionList = false;
	}
	else
	{
		bShowConditionList = true;
		Cast<UTextBlock>(HUDWidget->GetShowConditionsButton()->GetAllChildren()[0])->SetText(
			FText::FromString("Hide Conditions"));
		UScrollBox* ConditionList = HUDWidget->GetConditionList();
		ConditionList->ClearChildren();
		ConditionList->SetVisibility(ESlateVisibility::Visible);

		FString LastPhase = "";

		const TArray<USFCondition*> Conditions = USFGameInstance::Get()->GetParticipant()->GetAllConditions();

		for (const USFCondition* Condition : Conditions)
		{
			if (LastPhase != Condition->PhaseName)
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
