// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SFMasterHUD.h"
#include "HUD/SFConditionListEntry.h"
#include "SFGameInstance.h"
#include "SFParticipant.h"
#include "SFPlugin.h"
#include "Help/SFUtils.h"

#include "HeadMountedDisplayFunctionLibrary.h"


ASFMasterHUD::ASFMasterHUD()
{
}

void ASFMasterHUD::DrawHUD()
{
	if (FSFUtils::IsPrimary())
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

	if (USFGameInstance::Get()->GetStudySetup() && !USFGameInstance::Get()->GetExperimenterViewConfig().bShowHUD)
	{
		return;
	}

	if (!SFWidgetClass)
	{
		return;
	}

	HMDHUDHelper = nullptr;
	if (FSFUtils::IsHMD())
	{
		HMDHUDHelper = Cast<ASFHMDSpectatorHUDHelp>(
			GetWorld()->SpawnActor(ASFHMDSpectatorHUDHelp::StaticClass()));
		HUDWidget = Cast<USFHUDWidget>(HMDHUDHelper->CreateWidget(SFWidgetClass));
	}
	else if (FSFUtils::IsPrimary())
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
		else if (USFGameInstance::Get()->GetExperimenterViewConfig().bShowExperimenterViewInSecondWindow)
		{
			TSharedRef<SWidget> SlateWidget = HUDWidget->TakeWidget();
			USFGameInstance::Get()->GetExperimenterWindow()->AddHUDWidget(SlateWidget);
		}
		else
		{
			HUDWidget->AddToViewport();
		}
	}



	FHUDSavedData& Data = USFGameInstance::Get()->HUDSavedData;

	HUDWidget->SetData(Data);

	if(UnForwardedParticipant)
	{
		//UpdateHUD was called before BeginPlay()
		UpdateHUD(UnForwardedParticipant, UnforwardedStatus);
	}


	if (USFGameInstance::Get()->IsStarted())
	{
		SetStartStudyButtonVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		SetNextConditionButtonVisibility(ESlateVisibility::Collapsed);
	}

	if (USFGameInstance::Get()->GetParticipant() && USFGameInstance::Get()->GetParticipant()->GetAllConditions()[0]->IsFinished())
	{
		Cast<UTextBlock>(HUDWidget->GetStartButton()->GetAllChildren()[0])->SetText(
			FText::FromString("Continue Study"));
	}

	if(USFGameInstance::Get()->GetParticipant() && !USFGameInstance::Get()->GetParticipant()->GetNextCondition())
	{
		Cast<UTextBlock>(HUDWidget->GetNextButton()->GetAllChildren()[0])->SetText(
			FText::FromString("End Study"));
	}

	HUDWidget->GetConditionList()->SetVisibility(ESlateVisibility::Collapsed);

	HUDWidget->GetStartButton()->OnClicked.AddDynamic(this, &ASFMasterHUD::OnStartButtonPressed);
	HUDWidget->GetNextButton()->OnClicked.AddDynamic(this, &ASFMasterHUD::OnNextButtonPressed);
	HUDWidget->GetShowConditionsButton()->OnClicked.AddDynamic(this, &ASFMasterHUD::OnShowConditionsButtonPressed);

	if (USFGameInstance::Get()->GetStudySetup() && USFGameInstance::Get()->GetExperimenterViewConfig().bShowConditionsPanelByDefault)
	{
		OnShowConditionsButtonPressed();
	}
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
	if (HUDWidget == nullptr) {
		//store it and set it later on
		UnForwardedParticipant = Participant;
		UnforwardedStatus = Status;
		return;
	}

	HUDWidget->SetStatus(Status);

	if (Participant == nullptr){
		USFGameInstance::Get()->HUDSavedData = HUDWidget->GetData();
		return;
	}

	HUDWidget->SetParticipant(Participant->GetID());

	USFCondition* Condition = Participant->GetCurrentCondition();

	if(!Condition){
		USFGameInstance::Get()->HUDSavedData = HUDWidget->GetData();
		return;
	}

	HUDWidget->SetPhase(Condition->PhaseName);
	TMap<FString, FString> FactorLevels = Condition->FactorLevels;
	HUDWidget->SetCondition(FactorLevels);

	USFGameInstance::Get()->HUDSavedData = HUDWidget->GetData();
}

void ASFMasterHUD::AddLogMessage(const FString& Text)
{
	HUDWidget->AddLogMessage(Text);
}

void ASFMasterHUD::SetBackgroundAlpha(float Alpha)
{
	BackgroundAlpha = Alpha;
}

void ASFMasterHUD::SetStartStudyButtonVisibility(ESlateVisibility Visibility)
{
	if(!HUDWidget)
		return;
	HUDWidget->GetStartButton()->SetVisibility(Visibility);
}

void ASFMasterHUD::SetNextConditionButtonVisibility(ESlateVisibility Visibility)
{
	if(!HUDWidget)
		return;
	HUDWidget->GetNextButton()->SetVisibility(Visibility);
}

void ASFMasterHUD::DrawBackground()
{
	FLinearColor BackgroundColor = FLinearColor::Black;
	if(USFGameInstance::Get() && USFGameInstance::Get()->GetFadeHandler())
	{
		BackgroundColor = USFGameInstance::Get()->GetFadeHandler()->GetFadeConfig().FadeColor;
	}
	BackgroundColor.A = BackgroundAlpha;
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	DrawRect(BackgroundColor, 0.0f, 0.0f, ViewportSize.X, ViewportSize.Y);
}

void ASFMasterHUD::OnStartButtonPressed()
{
	USFGameInstance::Get()->StartStudy();
}

void ASFMasterHUD::OnNextButtonPressed()
{
	USFGameInstance::Get()->NextCondition(true); //force it to change, so use with care!
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
		bool bFirstUnfinishedConditionFound = false;

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
			if (!Condition->IsFinished() && !bFirstUnfinishedConditionFound)
			{
				bFirstUnfinishedConditionFound = true;
				ConditionList->ScrollWidgetIntoView(Entry, false, EDescendantScrollDestination::Center);
			}
		}
	}
}

const ASFHMDSpectatorHUDHelp* ASFMasterHUD::GetHUDHelper() {
	return HMDHUDHelper;
}

void ASFMasterHUD::SetHUDVisible(bool bVisible)
{
	if (bVisible) {
		HUDWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		HUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
