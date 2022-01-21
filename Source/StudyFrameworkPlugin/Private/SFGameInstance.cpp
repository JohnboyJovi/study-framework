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

	FSFUtils::Log("USFGameInstance::Init");

	GEngine->GameViewportClientClass = USFGlobalFadeGameViewportClient::StaticClass();

	GoToConditionSyncedEvent.Attach(this);

	Instance = this;

	if (ConditionToStartAtInit)
	{
		FSFUtils::Log(
			"Started on a map that was part of the last study, so start the study run for debug reasons from Init()");
		RestoreLastParticipantForDebugStart(ConditionToStartAtInit);
	}
	
	if(StudySetup){
		InitFadeHandler(StudySetup->FadeConfig);
	}
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

	FSFUtils::Log("USFGameInstance::OnWorldStart for "+NewWorld->GetName());

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
		if (!IsInitialized())
		{
			//this is potentially called before init was called
			ConditionToStartAtInit = FirstMapCondition;
		}
		else
		{
			FSFUtils::Log(
				"[USFGameInstance::OnWorldChanged] Started on a map that was part of the last study, so start the study run for debug reasons.");
			RestoreLastParticipantForDebugStart(FirstMapCondition);
		}
		return;
	}

	// otherwise check whether a setup is present
	TArray<AActor*> StudySetups;
	UGameplayStatics::GetAllActorsOfClass(NewWorld, ASFStudySetup::StaticClass(), StudySetups);

	FSFUtils::Log("Found "+FString::FromInt(StudySetups.Num())+" ASFStudySetup actors on this map.");

	if (StudySetups.Num() == 1)
	{
		PrepareWithStudySetup(Cast<ASFStudySetup>(StudySetups[0]));
		return;
	}

	FSFUtils::Log(
		"[USFGameInstance::OnWorldChanged] world started that neither contains exactly one SFStudySetup actor, nor is a level that is part of one of the conditions from the last study run!",
		true);
}

void USFGameInstance::RestoreLastParticipantForDebugStart(USFCondition* InStartCondition)
{
	const int ParticipantID = USFParticipant::GetLastParticipantId();
	Participant = NewObject<USFParticipant>(this,
	                                        FName(TEXT("Participant_") + FString::FromInt(ParticipantID)));
	Participant->Initialize(ParticipantID);
	Participant->LoadConditionsFromJson();

	StudySetup = USFParticipant::GetLastParticipantSetup();
	InitFadeHandler(StudySetup->FadeConfig);

	StartCondition = InStartCondition;
	StartStudy();

	//cannot directly fade in here, since Init might not be done, so we wait a second for safety
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

void USFGameInstance::InitFadeHandler(FFadeConfig FadeConfig)
{
	if (FadeHandler == nullptr)
	{
		FadeHandler = NewObject<USFFadeHandler>(GetTransientPackage(), "SFFadeHandler");
		FadeHandler->AddToRoot();
	}
	FadeHandler->SetFadeConfig(FadeConfig);
}

void USFGameInstance::PrepareWithStudySetup(ASFStudySetup* Setup)
{
	StudySetup = DuplicateObject(Setup, this);

	int ParticipantID = USFParticipant::GetLastParticipantId();
	TArray<USFCondition*> Conditions;
	if (USFParticipant::GetLastParticipantFinished())
	{
		ParticipantID++;
		Conditions = StudySetup->GetAllConditionsForRun(ParticipantID);
	}
	else
	{
		const FText MessageText = FText::FromString(
			FString("The last participant did not finish the study run. Would you like to:") +
			"\n[Retry] Retry last participant (Participant ID: " +
			FString::FromInt(ParticipantID) + ") where he/she left off in condition # " +
			FString::FromInt(USFParticipant::GetLastParticipantLastConditionStarted()) +
			"\n[Continue] Continue with the next participant (Participant ID: " + FString::FromInt(ParticipantID + 1) +
			")\n[Cancel] Restart the entire study anew (Participant ID: 0)");
		const FText MessageTitle = FText::FromString("WARNING: Unfinished study run detected");
		const EAppReturnType::Type Answer = FMessageDialog::Open(EAppMsgType::CancelRetryContinue, MessageText,
		                                                         &MessageTitle);

		switch (Answer)
		{
		case EAppReturnType::Cancel:
			FSFUtils::Log("[USFGameInstance::PrepareWithStudySetup]: Restart entire study");
			ParticipantID = 0;
			Conditions = StudySetup->GetAllConditionsForRun(ParticipantID);
			//clear data
			USFParticipant::ClearPhaseLongtables(Setup);
			break;
		case EAppReturnType::Retry:
			FSFUtils::Log("[USFGameInstance::PrepareWithStudySetup]: Retry last participant");
			Conditions = USFParticipant::GetLastParticipantsConditions();
			StartCondition = Conditions[USFParticipant::GetLastParticipantLastConditionStarted()];
			break;
		case EAppReturnType::Continue:
			FSFUtils::Log("[USFGameInstance::PrepareWithStudySetup]: Continue with the next participant");
			ParticipantID++;
			Conditions = StudySetup->GetAllConditionsForRun(ParticipantID);
			break;
		default: ;
		}
	}


	// Participant
	Participant = NewObject<USFParticipant>(this,
	                                        FName(TEXT("Participant_") + FString::FromInt(ParticipantID)));
	Participant->Initialize(ParticipantID);
	Participant->SetStudyConditions(Conditions);

	if (IsInitialized())
	{
		InitFadeHandler(Setup->FadeConfig);
	}
	UpdateHUD("Wait for Start");
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

	if (!StartCondition)
	{
		//we are actually doing a real start and not just a "debug-start"
		if (!Participant->StartStudy())
		{
			FSFUtils::Log("[USFGameInstance::StartStudy()]: unable to start study.", true);
			return false;
		}
	}

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
	USFCondition* LastCondition = Participant->GetCurrentCondition();
	if (LastCondition && LastCondition->WasStarted())
		LastCondition->End();

	Participant->EndStudy();

	UpdateHUD("Study ended");
	if(GetHUD()){
		GetHUD()->SetNextConditionButtonVisibility(ESlateVisibility::Collapsed);
	}

	FadeHandler->FadeOut();
}


bool USFGameInstance::NextCondition(bool bForced /*=false*/)
{
	USFCondition* NextCondition = Participant->GetNextCondition();
	if (!NextCondition)
	{
		EndStudy();
		return false;
	}
	return GoToCondition(NextCondition, bForced);
}

bool USFGameInstance::GoToCondition(const USFCondition* Condition, bool bForced /*=false*/)
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

	GoToConditionSyncedEvent.Send(Condition->UniqueName, bForced);
	return true;
}

void USFGameInstance::GoToConditionSynced(FString ConditionName, bool bForced)
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
	if (LastCondition && LastCondition->WasStarted())
	{
		if (!LastCondition->End())
		{
			if (bForced)
			{
				FSFUtils::Log(
					"[USFGameInstance::GoToCondition()]: Forced to go to next condition, but current one is not finished!",
					true);
			}
			else
			{
				FSFUtils::Log(
					"[USFGameInstance::GoToCondition()]: Cannot go to next condition, since current one is not finished!",
					true);
				return;
			}
		}
	}

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

	bStudyStarted = true;
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

void USFGameInstance::UpdateHUD(FString Status)
{
	if (GetHUD())
	{
		GetHUD()->UpdateHUD(Participant, Status);
	}
	else
	{
		HUDSavedData.Status = Status;
		if (Participant)
			HUDSavedData.Participant = FString::FromInt(Participant->GetID());
	}
}

ASFMasterHUD* USFGameInstance::GetHUD()
{
	if (GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		return Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	}
	return nullptr;
}

USFFadeHandler* USFGameInstance::GetFadeHandler()
{
	return FadeHandler;
}

ASFStudySetup* USFGameInstance::GetStudySetup()
{
	return StudySetup;
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
	UpdateHUD("Fading In");
}

void USFGameInstance::OnFadedIn()
{
	OnFadedInDelegate.Broadcast();

	Participant->GetCurrentCondition()->Begin();
	Participant->LogComment("Start Condition: " + Participant->GetCurrentCondition()->GetPrettyName());

	UpdateHUD("Condition Running");
}

USFParticipant* USFGameInstance::GetParticipant() const
{
	return Participant;
}
