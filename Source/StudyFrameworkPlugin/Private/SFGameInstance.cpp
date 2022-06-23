// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGameInstance.h"

#include "HUD/SFFadeHandler.h"
#include "HUD/SFMasterHUD.h"
#include "HUD/SFGlobalFadeGameViewportClient.h"

#include "Help/SFUtils.h"
#include "Logging/SFLoggingBPLibrary.h"
#include "Logging/SFLoggingUtils.h"

USFGameInstance* USFGameInstance::Instance = nullptr;

// ****************************************************************** // 
// ******* Initialization ******************************************* //
// ****************************************************************** //

void USFGameInstance::Init()
{
	Super::Init();

	FSFLoggingUtils::Log("USFGameInstance::Init");

	GEngine->GameViewportClientClass = USFGlobalFadeGameViewportClient::StaticClass();

	GoToConditionSyncedEvent.Attach(this);

	Instance = this;
	LogObject = NewObject<USFLogObject>();

	if (ConditionToStartAtInit)
	{
		FSFLoggingUtils::Log(
			"Started on a map that was part of the last study, so start the study run for debug reasons from Init()");
		RestoreLastParticipantForDebugStart(ConditionToStartAtInit);
	}

	if (StudySetup)
	{
		InitFadeHandler(StudySetup->FadeConfig);
	}

	// Register delegate for ticker callback
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &USFGameInstance::LogTick));
	LogObject->Initialize();
}

void USFGameInstance::Shutdown()
{
	GoToConditionSyncedEvent.Detach();
	if(ExperimenterWindow)
	{
		ExperimenterWindow->DestroyWindow();
	}
	Instance = nullptr;
	// Unregister ticker delegate
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void USFGameInstance::OnWorldStart()
{
	UWorld* NewWorld = GetWorld();
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

	FSFLoggingUtils::Log("USFGameInstance::OnWorldStart for " + NewWorld->GetName());

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
			FSFLoggingUtils::Log(
				"[USFGameInstance::OnWorldChanged] Started on a map that was part of the last study, so start the study run for debug reasons.");
			RestoreLastParticipantForDebugStart(FirstMapCondition);
			FSFLoggingUtils::LogToHUD("Start map "+NewWorld->GetName()+" for debugging!");
		}
		return;
	}

	// otherwise check whether a setup is present
	TArray<AActor*> StudySetups;
	UGameplayStatics::GetAllActorsOfClass(NewWorld, ASFStudySetup::StaticClass(), StudySetups);

	FSFLoggingUtils::Log("Found " + FString::FromInt(StudySetups.Num()) + " ASFStudySetup actors on this map.");

	if (StudySetups.Num() == 1)
	{
		PrepareWithStudySetup(Cast<ASFStudySetup>(StudySetups[0]));
		return;
	}

	// else we started on an unrelated map and just disable the HUD, so people can test their stuff
	bStartedOnUnrelatedMap = true;
	InitFadeHandler(FFadeConfig());
	FadeHandler->FadeIn();
	FSFLoggingUtils::Log(
		"[USFGameInstance::OnWorldChanged] world started that neither contains exactly one SFStudySetup actor, nor is a level that is part of one of the conditions from the last study run!",
		false);
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

	if(bStartedOnUnrelatedMap)
	{
		ExperimenterViewConfig.bShowHUD=false;
	}
	if(ExperimenterViewConfig.bShowExperimenterViewInSecondWindow)
	{
		ExperimenterWindow = NewObject<USFExperimenterWindow>(GetTransientPackage(), "ExperimenterWindow");
		ExperimenterWindow->CreateWindow(ExperimenterViewConfig);
	}
	
}

void USFGameInstance::PrepareWithStudySetup(ASFStudySetup* Setup)
{
	StudySetup = DuplicateObject(Setup, this);

	int ParticipantID = USFParticipant::GetLastParticipantId();
	TArray<USFCondition*> Conditions;
	bool bRecoverParticipantData = false;
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
			FSFLoggingUtils::Log("[USFGameInstance::PrepareWithStudySetup]: Restart entire study");
			ParticipantID = 0;
			Conditions = StudySetup->GetAllConditionsForRun(ParticipantID);
			//clear data
			USFParticipant::ClearPhaseLongtables(Setup);
			break;
		case EAppReturnType::Retry:
			FSFLoggingUtils::Log("[USFGameInstance::PrepareWithStudySetup]: Retry last participant");
			Conditions = USFParticipant::GetLastParticipantsConditions();
			StartCondition = Conditions[USFParticipant::GetLastParticipantLastConditionStarted()];
			bRecoverParticipantData = true;
			break;
		case EAppReturnType::Continue:
			FSFLoggingUtils::Log("[USFGameInstance::PrepareWithStudySetup]: Continue with the next participant");
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

	if (bRecoverParticipantData)
	{
		Participant->RecoverStudyResultsOfFinishedConditions();
	}

	ExperimenterViewConfig = StudySetup->ExperimenterViewConfig;

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
		FSFLoggingUtils::Log("[USFGameInstance::StartStudy()]: Study already started.", true);
		return false;
	}

	if (!StartCondition)
	{
		//we are actually doing a real start and not just a "debug-start"
		if (!Participant->StartStudy())
		{
			FSFLoggingUtils::Log("[USFGameInstance::StartStudy()]: unable to start study.", true);
			return false;
		}
	}

	if (GetWorld()->GetFirstPlayerController() && Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
	{
		ASFMasterHUD* HUD = Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
		HUD->SetStartStudyButtonVisibility(ESlateVisibility::Collapsed);
		HUD->SetNextConditionButtonVisibility(ESlateVisibility::Visible);
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
	if (GetHUD())
	{
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
		FSFLoggingUtils::Log("[USFGameInstance::GoToCondition()]: Already Fading between levels", true);
		return false;
	}

	if (!Condition || Condition->Map.Equals(""))
	{
		FSFLoggingUtils::Log("[USFGameInstance::GoToCondition()]: Could not load next condition.", true);
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
				FSFLoggingUtils::Log(
					"[USFGameInstance::GoToCondition()]: Forced to go to next condition, but current one is not finished!",
					true);
			}
			else
			{
				FSFLoggingUtils::Log(
					"[USFGameInstance::GoToCondition()]: Cannot go to next condition, since current one is not finished!",
					true);
				return;
			}
		}
	}

	bool bConditionPresent = Participant->SetCondition(NextCondition);
	if (!bConditionPresent)
	{
		FSFLoggingUtils::Log("[USFGameInstance::GoToCondition()]: Requested GoTo Condition seems not to exist.", true);
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
	if(!Participant)
	{
		return "ParticipantNotSet";
	}
	if (Participant->GetCurrentCondition()->FactorLevels.Contains(FactorName))
	{
		return Participant->GetCurrentCondition()->FactorLevels[FactorName];
	}

	FSFLoggingUtils::Log("[USFGameInstance::GetFactorLevel()]: Factor " + FactorName + " seems not to exist!", true);
	return "FactorNotPresent";
}

void USFGameInstance::LogComment(const FString& Comment)
{
	FSFLoggingUtils::LogComment(Comment);
	FSFLoggingUtils::LogToHUD(Comment);
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

FExperimenterViewConfig USFGameInstance::GetExperimenterViewConfig() const
{
	return ExperimenterViewConfig;
}

USFExperimenterWindow* USFGameInstance::GetExperimenterWindow() const
{
	return ExperimenterWindow;
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

void USFGameInstance::OnLevelLoaded()
{
	//do we need to do something here?
	UpdateHUD("Fading In");
}

void USFGameInstance::OnFadedIn()
{
	OnFadedInDelegate.Broadcast();

	if(bStartedOnUnrelatedMap)
	{
		return;
	}

	Participant->GetCurrentCondition()->Begin();
	FSFLoggingUtils::LogComment("Start Condition: " + Participant->GetCurrentCondition()->GetPrettyName());

	UpdateHUD("Condition "+FString::FromInt(Participant->GetCurrentConditionNumber()+1)+"/"+FString::FromInt(Participant->GetAllConditions().Num()));
}

USFParticipant* USFGameInstance::GetParticipant() const
{
	return Participant;
}

// ****************************************************************** // 
// ******* Logging ************************************************** //
// ****************************************************************** //
bool USFGameInstance::LogTick(float DeltaSeconds)
{
	/*if(UseLogging)
	{*/
		USFLoggingBPLibrary::LogToFile();
	//}
		return true;
}

USFLogObject* USFGameInstance::GetLogObject()
{
	return LogObject;
}