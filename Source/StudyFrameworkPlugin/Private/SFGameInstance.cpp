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

	if (!StudySetup)
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

	//TODO: reactivate the start study button?
}


bool USFGameInstance::NextCondition()
{
	USFCondition* NextCondition = Participant->NextCondition();
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

	USFCondition* LastCondition = Participant->GetCurrentCondition();
	LastCondition->End();

	bool bConditionPresent = Participant->SetCondition(Condition);
	if (!bConditionPresent)
	{
		FSFUtils::Log("[USFGameInstance::GoToCondition()]: Requested GoTo Condition seems not to exist.", true);
		return false;
	}

	// Fade to next Level
	FadeHandler->FadeToLevel(Condition->Map);
	UpdateHUD("Fading out");
	return true;
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
