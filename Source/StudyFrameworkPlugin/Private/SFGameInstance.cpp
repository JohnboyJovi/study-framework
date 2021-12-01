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

	Instance = this;

	GoToConditionSyncedEvent.Attach(this);
}

void USFGameInstance::Shutdown()
{
	GoToConditionSyncedEvent.Detach();
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

	if(!Instance)
	{
		//this is potentially called before init was called
		Init();
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

	if (FirstMapCondition)
	{
		FSFUtils::Log("Started on a map that was part of the last study, so start the study run for debug resaons");
		Initialize(USFParticipant::GetLastParticipantId());
		StartStudy(FirstMapCondition);
	}
}


USFGameInstance* USFGameInstance::Get()
{
	if (Instance == nullptr)
	{
		FSFUtils::OpenMessageBox("GameInstance is not set to USFGameInstance, Study Framework will not work", true);
	}
	return Instance;
}

bool USFGameInstance::IsGameInstanceSet()
{
	return Instance != nullptr;
}

void USFGameInstance::Initialize(int ParticipantID, FString JsonFilePath)
{
	if (bInitialized)
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
	Participant->Initialize(ParticipantID, JsonFilePath);

	bInitialized = true;
}


bool USFGameInstance::IsInitialized() const
{
	return bInitialized;
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
	Participant->CommitData();
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

	// Commit data at SFLogger
	CommitData();

	if (!Condition || Condition->Map.Equals(""))
	{
		FSFUtils::Log("[USFGameInstance::GoToCondition()]: Could not load next condition.", true);
		return false;
	}

	GoToConditionSyncedEvent.Send(Condition->GetName());
	return true;
}

void USFGameInstance::GoToConditionSynced(FString ConditionName)
{
	USFCondition* NextCondition = nullptr;
	for (USFCondition* Condition : Participant->GetAllConditions())
	{
		if (Condition->GetName() == ConditionName)
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
	FadeHandler->FadeToLevel(NextCondition->Map);
	UpdateHUD("Fading out");
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


void USFGameInstance::SaveData(const FString Where, FString Data)
{
	TArray<FString> DataArray;
	DataArray.Add(Data);

	SaveDataArray(Where, DataArray);
}

void USFGameInstance::SaveDataArray(const FString Where, TArray<FString> Data)
{
	Participant->SaveDataArray(Where, Data);
}

void USFGameInstance::CommitData()
{
	Participant->CommitData();
}

void USFGameInstance::LogData(const FString String)
{
	Participant->LogData(String);
	LogToHUD(String);
	//TODO: log it to HUD here?
}

void USFGameInstance::LogToHUD(FString Text)
{
	ASFMasterHUD* MasterHUD = Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	if (MasterHUD && MasterHUD->IsWidgetPresent())
	{
		MasterHUD->AddLogMessage(Text);
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
