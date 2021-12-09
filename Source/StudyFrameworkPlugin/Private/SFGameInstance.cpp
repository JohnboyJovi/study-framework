// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGameInstance.h"

#include "HUD/SFFadeHandler.h"
#include "HUD/SFMasterHUD.h"
#include "HUD/SFGlobalFadeGameViewportClient.h"

#include "Help/SFUtils.h"

USFGameInstance* USFGameInstance::Instance = nullptr;


// ****************************************************************** // 
// ******* Initialization ******************************************* //
// ****************************************************************** //

void USFGameInstance::Init()
{
	Super::Init();

	GEngine->GameViewportClientClass = USFGlobalFadeGameViewportClient::StaticClass();

	GoToConditionSyncedEvent.Attach(this);

	if (ConditionToStartAtInit)
	{
		FSFUtils::Log(
			"Started on a map that was part of the last study, so start the study run for debug reasons from Init()");
		RestoreLastParticipant(ConditionToStartAtInit);
	}

	Instance = this;
}

void USFGameInstance::Shutdown()
{
	GoToConditionSyncedEvent.Detach();
	Instance = nullptr;
}

void USFGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);

	//when unloading map, NewWorld==nullptr, when loading new map OldWorld==nullptr
	if (NewWorld == nullptr)
	{
		return;
	}

	if (bStudyStarted)
	{
		//everything is already up and running, so nothing to do
		return;
	}

	// so we have loaded a new world and the study is not running, so check whether this is a map in one of the conditions
	TArray<USFCondition*> LastConditions = USFParticipant::GetLastParticipantsConditions();
	USFCondition* FirstMapCondition = nullptr;
	for (USFCondition* Condition : LastConditions)
	{
		if (FPaths::GetBaseFilename(Condition->Map).Equals(NewWorld->GetName()))
		{
			FirstMapCondition = Condition;
			break;
		}
	}

	if (!IsInitialized())
	{
		//this is potentially called before init was called
		ConditionToStartAtInit = FirstMapCondition;
	}
	else if (FirstMapCondition)
	{
		FSFUtils::Log("Started on a map that was part of the last study, so start the study run for debug reasons");
		RestoreLastParticipant(FirstMapCondition);
	}
}

void USFGameInstance::RestoreLastParticipant(USFCondition* StartCondition)
{
	PrepareForParticipant(USFParticipant::GetLastParticipantId());
	Participant->LoadConditionsFromJson();
	StartStudy(StartCondition);
	GetTimerManager().SetTimer(StartFadingTimerHandle, this, &USFGameInstance::StartFadingIn, 1.0f);
}

void USFGameInstance::StartFadingIn()
{
	FadeHandler->FadeIn();
	GetTimerManager().ClearTimer(StartFadingTimerHandle);
}


USFGameInstance* USFGameInstance::Get()
{
	if (Instance == nullptr)
	{
		FSFUtils::OpenMessageBox("GameInstance is not set to USFGameInstance, Study Framework will not work", true);
	}
	return Instance;
}

bool USFGameInstance::IsInitialized()
{
	return Instance != nullptr;
}

void USFGameInstance::PrepareForParticipant(int ParticipantID)
{
	if (bPrepared)
	{
		return;
	}

	// FadeHandler
	FadeHandler = NewObject<USFFadeHandler>(GetTransientPackage(), "SFFadeHandler");
	FadeHandler->AddToRoot();
	FadeHandler->SetGameInstance(this);
	FadeHandler->SetInitialFadedOut(false);

	// Participant
	Participant = NewObject<USFParticipant>(GetTransientPackage(),
	                                        FName(TEXT("Participant_") + FString::FromInt(ParticipantID)));
	Participant->Initialize(ParticipantID);

	bPrepared = true;
}


bool USFGameInstance::IsPrepared() const
{
	return bPrepared;
}


// ****************************************************************** // 
// ******* Control Study ******************************************** //
// ****************************************************************** //

bool USFGameInstance::StartStudy(const USFCondition* StartCondition /*= nullptr*/)
{
	if (bStudyStarted)
	{
		FSFUtils::Log("[USFGameInstance::StartStudy()]: Study already started.", true);
		return false;
	}

	if (!StartCondition)
	{
		//we are actually doing a real start and not just a "debug-start"
		if (!StudySetup)
		{
			FSFUtils::OpenMessageBox("[USFGameInstance::StartStudy()]: Not StudySetup specified. Please do so.", true);
		}

		if (!Participant->StartStudy(StudySetup))
		{
			FSFUtils::Log("[USFGameInstance::StartStudy()]: unable to start study.", true);
			return false;
		}
	}

	bStudyStarted = true;

	if (GetWorld()->GetFirstPlayerController() && Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
	{
		Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())->SetStartStudyButtonVisibility(
			ESlateVisibility::Collapsed);
	}


	if (!StartCondition)
	{
		NextCondition();
	}
	else
	{
		GoToCondition(StartCondition);
	}

	return true;
}

void USFGameInstance::EndStudy()
{
	Participant->EndStudy();

	UpdateHUD("Study ended");

	//TODO: reactivate the start study button?
}


bool USFGameInstance::NextCondition()
{
	USFCondition* NextCondition = Participant->GetNextCondition();
	if (!NextCondition)
	{
		EndStudy();
		return false;
	}
	return GoToCondition(NextCondition);
}

bool USFGameInstance::GoToCondition(const USFCondition* Condition)
{
	// Check if is already fading
	if (FadeHandler->GetIsFading())
	{
		FSFUtils::Log("[USFGameInstance::GoToCondition()]: Already Fading between levels", true);
		return false;
	}

	if (!Condition || Condition->Map.Equals(""))
	{
		FSFUtils::Log("[USFGameInstance::GoToCondition()]: Could not load next condition.", true);
		return false;
	}

	GoToConditionSyncedEvent.Send(Condition->UniqueName);
	return true;
}

void USFGameInstance::GoToConditionSynced(FString ConditionName)
{
	USFCondition* NextCondition = nullptr;
	for (USFCondition* Condition : Participant->GetAllConditions())
	{
		if (Condition->UniqueName == ConditionName)
		{
			NextCondition = Condition;
		}
	}

	USFCondition* LastCondition = Participant->GetCurrentCondition();
	if (LastCondition)
		LastCondition->End();

	bool bConditionPresent = Participant->SetCondition(NextCondition);
	if (!bConditionPresent)
	{
		FSFUtils::Log("[USFGameInstance::GoToCondition()]: Requested GoTo Condition seems not to exist.", true);
		return;
	}

	// Fade to next Level
	if (IsInitialized())
	{
		FadeHandler->FadeToLevel(NextCondition->Map);
		UpdateHUD("Fading out");
	}
}

bool USFGameInstance::IsStarted() const
{
	return bStudyStarted;
}

FString USFGameInstance::GetFactorLevel(FString FactorName) const
{
	if (Participant->GetCurrentCondition()->FactorLevels.Contains(FactorName))
	{
		return Participant->GetCurrentCondition()->FactorLevels[FactorName];
	}

	FSFUtils::Log("[USFGameInstance::GetFactorLevel()]: Factor " + FactorName + " seems not to exist!", true);
	return "FactorNotPresent";
}


void USFGameInstance::LogData(const FString& DependenVariableName, const FString& Value)
{
	Participant->LogData(DependenVariableName, Value);
}

void USFGameInstance::LogComment(const FString& Comment)
{
	Participant->LogComment(Comment);
	LogToHUD(Comment);
}

void USFGameInstance::LogToHUD(FString Text)
{
	if (GetWorld()->GetFirstPlayerController() && Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()) &&
		Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())->IsWidgetPresent())
	{
		Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())->AddLogMessage(Text);
	}
	else
	{
		HUDSavedData.LogMessages.Add(Text);
	}
}


// ****************************************************************** // 
// ******* Prepare Study ******************************************** //
// ****************************************************************** //

USFStudySetup* USFGameInstance::CreateNewStudySetup()
{
	if (bStudyStarted)
	{
		FSFUtils::Log("[USFGameInstance::CreateNewStudySetup()]: Study already started.", true);
	}

	StudySetup = NewObject<USFStudySetup>(GetTransientPackage(), "StudySetup");
	return StudySetup;
}

USFStudySetup* USFGameInstance::GetStudySetup()
{
	return StudySetup;
}

void USFGameInstance::LoadStudySetupFromJson()
{
	FSFUtils::OpenMessageBox("USFGameInstance::LoadStudySetupFromJson not implemented!", true);
}

void USFGameInstance::SaveStudySetupToJson(FString Filename) const
{
	TSharedPtr<FJsonObject> Json = StudySetup->GetAsJson();
	FSFUtils::WriteJsonToFile(Json, Filename);
}

USFFadeHandler* USFGameInstance::GetFadeHandler()
{
	return FadeHandler;
}

// ****************************************************************** // 
// ******* Executing Study ****************************************** //
// ****************************************************************** //

void USFGameInstance::SpawnAllActorsForLevel()
{
	// Spawn all level specific actor
	TArray<TSubclassOf<AActor>> SpawnInThisPhase = Participant->GetCurrentCondition()->SpawnInThisCondition;
	for (auto EntryC : SpawnInThisPhase)
	{
		GetWorld()->SpawnActor(EntryC);
	}
}

void USFGameInstance::OnLevelLoaded()
{
	//do we need to do something here?
	//Participant->GetCurrentPhase()->ApplyCondition();
	UpdateHUD("Fading In");
}

void USFGameInstance::OnFadedIn()
{
	OnFadedInDelegate.Broadcast();

	Participant->GetCurrentCondition()->Begin();
	Participant->LogComment("Started: "+Participant->GetCurrentCondition()->GetPrettyName());

	UpdateHUD("Condition Running");
}

USFParticipant* USFGameInstance::GetParticipant() const
{
	return Participant;
}


void USFGameInstance::UpdateHUD(FString Status)
{
	ASFMasterHUD* MasterHUD = Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	if (MasterHUD)
		MasterHUD->UpdateHUD(Participant, Status);
}
