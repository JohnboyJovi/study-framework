// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGameInstance.h"

#include <string>

#include "SFFadeHandler.h"
#include "SFUtils.h"

#include "EngineUtils.h"            // For Spawning in Actor in each level
#include "IUniversalLogging.h"
#include "SFMasterHUD.h"
#include "SFGlobalFadeGameViewportClient.h"

USFGameInstance* USFGameInstance::Instance = nullptr;


// ****************************************************************** // 
// ******* Initialization ******************************************* //
// ****************************************************************** //

void USFGameInstance::Init()
{
	Super::Init();

	GEngine->GameViewportClientClass = USFGlobalFadeGameViewportClient::StaticClass();

	Instance=this;
}

USFGameInstance* USFGameInstance::Get()
{
	if (Instance == nullptr)
	{
		FSFUtils::OpenMessageBox("GameInstance is not set to USFGameInstance, Study Framework will not work", true);
	}
	return Instance;
}

void USFGameInstance::Initialize(FString ParticipantID, FString JsonFilePath)
{
	if (bInitialized)
	{
		return;
	}

	// FadeHandler
	FadeHandler = NewObject<USFFadeHandler>(GetTransientPackage(), "SFFadeHandler");
	FadeHandler->AddToRoot(); // TODO What is that?
	FadeHandler->SetGameInstance(this);
	FadeHandler->SetInitialFadedOut(false);

	// Participant
	Participant = NewObject<USFParticipant>(GetTransientPackage(), FName(*ParticipantID));
	Participant->Initialize(ParticipantID, JsonFilePath);

	bInitialized = true;

	UpdateHUD("Wait for start");
}


bool USFGameInstance::IsInitialized() const
{
	return bInitialized;
}


// ****************************************************************** // 
// ******* Control Study ******************************************** //
// ****************************************************************** //

bool USFGameInstance::StartStudy()
{
	if (bStudyStarted)
	{
		FSFUtils::Log("[USFGameInstance::StartStudy()]: Study already started.", true);
		return false;
	}

	if(!StudySetup)
	{
		FSFUtils::OpenMessageBox("[USFGameInstance::StartStudy()]: Not StudySetup specified. Please do so.", true);
	}

	if (!Participant->StartStudy(StudySetup))
	{
		FSFUtils::Log("[USFGameInstance::StartStudy()]: unable to start study.", true);
		return false;
	}

	bStudyStarted = true;

	NextCondition();
	return true;
}

void USFGameInstance::EndStudy()
{
	Participant->CommitData();
	Participant->EndStudy();

	UpdateHUD("Study ended");
}


bool USFGameInstance::NextCondition()
{
	// Check if is already fading
	if (FadeHandler->GetIsFading())
	{
		FSFUtils::Log("[USFGameInstance::NextCondition()]: Already Fading between levels", true);
		return false;
	}

	// Commit data at SFLogger
	Participant->CommitData();

	FString NextLevelName = Participant->NextCondition();

	if (NextLevelName.Equals(""))
	{
		FSFUtils::Log("[USFGameInstance::NextCondition()]: Could not load next setup.", true);
		return false;
	}

	// Fade to next Level
	FadeHandler->FadeToLevel(NextLevelName);
	UpdateHUD("Fading out");
	return true;
}

bool USFGameInstance::IsStarted() const
{
	return bStudyStarted;
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
	if (MasterHUD)
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

	StudySetup =  NewObject<USFStudySetup>(GetTransientPackage(), "StudySetup");
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

void USFGameInstance::AddActorForEveryLevelInEveryPhaseCpp(UClass* Actor)
{
	SpawnInEveryPhaseCpp.Add(Actor);
}

void USFGameInstance::AddActorForEveryLevelInEveryPhaseBlueprint(const FSFClassOfBlueprintActor Actor)
{
	SpawnInEveryPhaseBlueprint.Add(Actor);
}


void USFGameInstance::SetFadeColor(const FLinearColor Color)
{
	FadeHandler->SetFadeColor(Color);
}

void USFGameInstance::SetFadeDuration(const float FadeDuration)
{
	FadeHandler->SetFadeDuration(FadeDuration);
}

void USFGameInstance::SetFadedOutDuration(const float FadeOutWait)
{
	FadeHandler->SetFadedOutDuration(FadeOutWait);
}

void USFGameInstance::SetInitialFadedOut(const bool bFadedOut)
{
	FadeHandler->SetInitialFadedOut(bFadedOut);
}


// ****************************************************************** // 
// ******* Executing Study ****************************************** //
// ****************************************************************** //

void USFGameInstance::SpawnAllActorsForLevel()
{
	// Spawn all for every level
	for (auto EntryC : SpawnInEveryPhaseCpp)
	{
		GetWorld()->SpawnActor(EntryC);
	}

	for (auto EntryBlueprint : SpawnInEveryPhaseBlueprint)
	{
		SpawnBlueprintActor(EntryBlueprint);
	}

	// Spawn all level specific actor
	TArray<UClass*> SpawnInThisPhaseCpp = Participant->GetCurrentPhase()->GetSpawnActorsCpp();
	for (auto EntryC : SpawnInThisPhaseCpp)
	{
		GetWorld()->SpawnActor(EntryC);
	}

	TArray<FSFClassOfBlueprintActor> SpawnInThisPhaseBlueprint = Participant
	                                                             ->GetCurrentPhase()->GetSpawnActorsBlueprint();
	for (auto EntryBlueprint : SpawnInThisPhaseBlueprint)
	{
		SpawnBlueprintActor(EntryBlueprint);
	}
}

void USFGameInstance::SpawnBlueprintActor(const FSFClassOfBlueprintActor Actor) const
{
	FString ClassNameC = Actor.ClassName;
	ClassNameC.Append(FString("_C"));

	TArray<UObject*> TmpArray;

	if (EngineUtils::FindOrLoadAssetsByPath(*Actor.Path, TmpArray, EngineUtils::ATL_Class))
	{
		for (int i = 0; i < TmpArray.Num(); ++i)
		{
			UObject* Tmp = TmpArray[i];
			if (Tmp == nullptr || (!dynamic_cast<UClass*>(Tmp)) || (Tmp->GetName().Compare(ClassNameC) != 0))
			{
				continue;
			}

			GetWorld()->SpawnActor(dynamic_cast<UClass*>(Tmp));
			return;
		}
	}

	FSFUtils::Log("[USFGameInstance::SpawnBlueprintActor()]: Unable to spawn blueprint actor ("
	              + Actor.Path + "/" + Actor.ClassName + ") cannot be found!", true);
}

void USFGameInstance::OnLevelLoaded()
{
	Participant->GetCurrentPhase()->ApplyCondition();
	UpdateHUD("Fading In");
}

void USFGameInstance::OnFadedIn()
{
	OnFadedInDelegate.Broadcast();

	UpdateHUD("Condition Running");
}


void USFGameInstance::UpdateHUD(FString Status)
{
	ASFMasterHUD* MasterHUD = Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	if (MasterHUD)
		MasterHUD->UpdateHUD(Participant, Status);
}
