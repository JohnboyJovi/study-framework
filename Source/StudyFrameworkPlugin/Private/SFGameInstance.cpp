// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGameInstance.h"

#include "IUniversalLogging.h"
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

	bStudyStarted = false;
	bStudyEnded = false;
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
			LogToHUD("Start map "+NewWorld->GetName()+" for debugging!");
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
	const int ParticipantSequenceNumber = USFParticipant::GetLastParticipantSequenceNumber();
	const FString ParticipantID = USFParticipant::GetLastParticipantID();
	Participant = NewObject<USFParticipant>(this,
	                                        FName(TEXT("Participant_") + ParticipantID));
	
	StudySetup = USFParticipant::GetLastParticipantSetup();

	Participant->Initialize(ParticipantSequenceNumber, ParticipantID);
	Participant->LoadConditionsFromJson();

	
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
		StudySetup->ExperimenterViewConfig.bShowHUD=false;
	}
	if(StudySetup->ExperimenterViewConfig.bShowExperimenterViewInSecondWindow)
	{
		ExperimenterWindow = NewObject<USFExperimenterWindow>(GetTransientPackage(), "ExperimenterWindow");
		ExperimenterWindow->CreateWindow(StudySetup->ExperimenterViewConfig);
	}
	
}

void USFGameInstance::PrepareWithStudySetup(ASFStudySetup* Setup)
{
	StudySetup = DuplicateObject(Setup, this);

	int ParticipantSequenceNumber = USFParticipant::GetLastParticipantSequenceNumber();
	FString LastParticipantID = USFParticipant::GetLastParticipantID();
	TArray<USFCondition*> Conditions;
	bool bRecoverParticipantData = false;
	if (USFParticipant::GetLastParticipantFinished())
	{
		ParticipantSequenceNumber++;
		Conditions = StudySetup->GetAllConditionsForRun(ParticipantSequenceNumber);
	}
	else
	{

		const FString MessageText = FString("The last participant did not finish the study run. Would you like to:") +
			"\n[Continue Participant] Continue last participant (Participant ID: " +
			LastParticipantID + ", SequenceNumber: " + FString::FromInt(ParticipantSequenceNumber) + ") where he/she left off in condition # " +
			FString::FromInt(USFParticipant::GetLastParticipantLastConditionStarted()) +
			"\n[Next Participant] Continue with the next participant (ParticipantSequenceNumber: " + FString::FromInt(ParticipantSequenceNumber + 1) +
			")\n[Restart Study] Restart the entire study anew (ParticipantSequenceNumber: 0)";
		const FString MessageTitle = "WARNING: Unfinished study run detected";
		TArray<FString> Buttons = {
			"Continue Participant",
			"Next Participant",
			"Restart Study"
		};
		int Answer = FSFUtils::OpenCustomDialog(MessageTitle, MessageText, Buttons);
		
		switch (Answer)
		{
		case 2:
			FSFLoggingUtils::Log("[USFGameInstance::PrepareWithStudySetup]: Restart entire study");
			ParticipantSequenceNumber = 0;
			Conditions = StudySetup->GetAllConditionsForRun(ParticipantSequenceNumber);
			//clear data
			USFParticipant::ClearPhaseLongtables(Setup);
			break;
		case 0:
			FSFLoggingUtils::Log("[USFGameInstance::PrepareWithStudySetup]: Continue last participant");
			Conditions = USFParticipant::GetLastParticipantsConditions();
			StartCondition = Conditions[USFParticipant::GetLastParticipantLastConditionStarted()];
			bRecoverParticipantData = true;
			break;
		case 1:
			FSFLoggingUtils::Log("[USFGameInstance::PrepareWithStudySetup]: Continue with the next participant");
			ParticipantSequenceNumber++;
			Conditions = StudySetup->GetAllConditionsForRun(ParticipantSequenceNumber);
			break;
		default: ;
		}
	}

	if (StudySetup->bPreloadAllMapsOnStart)
	{
		PreloadAllMaps(Conditions);
	}


	// Participant
	FString ParticipantID = "";
	if(Setup->bUseCustomParticipantIDs)
	{
		//ask for it:
		bool bValidIDEntered = false;
		while(!bValidIDEntered)
		{
			FString IDGiven;
			int Result = FSFUtils::OpenCustomDialogText("Participant ID", "Please type in your ID:", "", IDGiven);
			if (Result < 0) {
				FSFLoggingUtils::Log("[USFGameInstance::PrepareWithStudySetup] The window for entering the participant ID was closed without giving an answer, repeat question!", false);
				continue;
			}
			//check whether it was used before
			if(USFParticipant::WasParticipantIdAlreadyUsed(IDGiven))
			{
				FSFUtils::OpenMessageBox("Participant ID \"" + IDGiven + "\"was already used, you have to choose another one!");
			}
			else
			{
				ParticipantID = IDGiven;
				bValidIDEntered = true;
			}
		}
	}
	else
	{
		//otherwise just use sequence number:
		ParticipantID = FString::FromInt(ParticipantSequenceNumber);
	}

	Participant = NewObject<USFParticipant>(this, FName(TEXT("Participant_") + ParticipantID));
	Participant->Initialize(ParticipantSequenceNumber, ParticipantID);
	if (bRecoverParticipantData) {
		Participant->LoadLastParticipantsIndependentVariables();
	}
	else {
		Participant->SetIndependentVariablesFromStudySetup(Setup);
		Participant->StoreInIndependentVarLongTable();
	}
	Participant->SetStudyConditions(Conditions);

	if (bRecoverParticipantData)
	{
		Participant->RecoverStudyResultsOfFinishedConditions();
	}


	if (IsInitialized())
	{
		InitFadeHandler(Setup->FadeConfig);
	}
	UpdateHUD("Wait for Start");
}

void USFGameInstance::PreloadAllMaps(const TArray<USFCondition*>& Conditions)
{
	TArray<FString> Maps;
	for(const USFCondition* Condition : Conditions)
	{
		if(!Maps.Contains(Condition->Map))
		{
			Maps.Add(Condition->Map);
		}
	}

	for(FString Map : Maps)
	{
		LoadPackage(nullptr, *Map, ELoadFlags::LOAD_None);
	}
	FSFLoggingUtils::Log("Sucessfully preloaded all maps.");
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

	//check whether we want to use the eye tracker
	if(StudySetup && StudySetup->UseGazeTracker != EGazeTrackerMode::NotTracking)
	{
		GazeTracker = NewObject<USFGazeTracker>(this, TEXT("GazeTracker"));
		GazeTracker->Init(StudySetup->UseGazeTracker, StudySetup->bIgnoreNonGazeTargetActors );
	}


	return true;
}

void USFGameInstance::EndStudy()
{
	USFCondition* LastCondition = Participant->GetCurrentCondition();
	if (!LastCondition || !LastCondition->WasStarted() || LastCondition->IsFinished())
		return;
		
	LastCondition->End();
	Participant->EndStudy();

	UpdateHUD("Study ended");
	if (GetHUD())
	{
		GetHUD()->SetNextConditionButtonVisibility(ESlateVisibility::Collapsed);
	}

	bStudyEnded = true;
	FadeHandler->FadeOut();
}


bool USFGameInstance::NextCondition(bool bForced /*=false*/)
{
	// Check if is already fading
	if (FadeHandler->GetIsFading() && !FadeHandler->GetIsFadedOutWaitingForLevel())
	{
		FSFLoggingUtils::Log("[USFGameInstance::NextCondition()]: Already Fading between levels", true);
		return false;
	}

	USFCondition* NextCondition = Participant->GetNextCondition();
	if (!NextCondition)
	{
		FSFLoggingUtils::Log("[SFGameInstance::NextCondition]: All conditions already ran, no NextCondition", false);
		EndStudy();
		return false;
	}
	return GoToCondition(NextCondition, bForced);
}

bool USFGameInstance::GoToCondition(const USFCondition* Condition, bool bForced /*=false*/)
{
	// Check if is already fading
	if (FadeHandler->GetIsFading() && !FadeHandler->GetIsFadedOutWaitingForLevel())
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
		TArray<FString> UnfinishedVars = LastCondition->End();
		if (UnfinishedVars.Num() != 0)
		{
			FString Missing = "Missing data for: {";
			for(FString UnfinishedVar : UnfinishedVars)
			{
				Missing += UnfinishedVar + " ";
			}
			Missing += "}";
			if (bForced)
			{
				FSFLoggingUtils::Log(
					"[USFGameInstance::GoToCondition()]: Forced to go to next condition, but current one is not finished! " + Missing,
					true);
			}
			else
			{
				FSFLoggingUtils::Log(
					"[USFGameInstance::GoToCondition()]: Cannot go to next condition, since current one is not finished! " + Missing,
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
		if(FadeHandler->GetIsFadedOutWaitingForLevel())
		{
			FadeHandler->SetLevelToLoad(NextCondition->Map);
		}
		else
		{
			FadeHandler->FadeToLevel(NextCondition->Map);
		}
		UpdateHUD("Fading out");
	}

	bStudyStarted = true;
}

bool USFGameInstance::IsStarted() const
{
	return bStudyStarted;
}

bool USFGameInstance::HasEnded() const
{
	return bStudyEnded;
}

float USFGameInstance::GetCurrentTime() const
{
	if(GetParticipant() && IsStarted())
	{
		return GetParticipant()->GetCurrentTime();
	}
	return 0.0f;
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

FString USFGameInstance::GetIndependentVariableValue(FString VariableName) const
{
	if(!GetParticipant())
	{
		return "";
	}
	return GetParticipant()->GetIndependentVariable(VariableName).Value;
}

FString USFGameInstance::GetCurrentPhase() const
{
	if (!Participant)
	{
		return "ParticipantNotSet";
	}

	if (!Participant->GetCurrentCondition())
	{
		return "NoConditionActive";
	}
	return Participant->GetCurrentCondition()->PhaseName;
}

int USFGameInstance::GetCurrentConditionsSequenceNumber() const
{
	if (!Participant) return -1;
	return Participant->GetCurrentConditionNumber() + 1;
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
	//by default we also log this into the debug stream, so you can also check on older messages
	FSFLoggingUtils::Log("[Logged to HUD] " + Text);
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
			HUDSavedData.Participant = Participant->GetID();
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

USFGazeTracker* USFGameInstance::GetGazeTracker() const
{
	return GazeTracker;
}

FExperimenterViewConfig USFGameInstance::GetExperimenterViewConfig() const
{
	return StudySetup->ExperimenterViewConfig;
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
	if(Participant && Participant->GetCurrentCondition())
	{
		//this should be logged first
		USFLoggingBPLibrary::LogComment("Start Condition: " + Participant->GetCurrentCondition()->GetPrettyName());
	}

	OnFadedInDelegate.Broadcast();

	if(bStartedOnUnrelatedMap)
	{
		return;
	}

	Participant->GetCurrentCondition()->Begin();

	UpdateHUD("Condition "+FString::FromInt(GetCurrentConditionsSequenceNumber())+"/"+FString::FromInt(Participant->GetAllConditions().Num()));
}

USFParticipant* USFGameInstance::GetParticipant() const
{
	return Participant;
}

// ****************************************************************** // 
// *******    Logging    ******************************************** //
// ****************************************************************** //

bool USFGameInstance::LogTick(float DeltaSeconds)
{
	if (LogObject->GetLoggingLoopsActive()){
		LogObject->WritePositionLogToFile();
		LogObject->WriteGazeTrackingLogToFile();
	}	
	return true;
}

USFLogObject* USFGameInstance::GetLogObject()
{
	return LogObject;
}

void USFGameInstance::LogComment(const FString& Comment, bool bAlsoLogToHUD /*= true*/)
{
	UniLog.Log("#" + GetParticipant()->GetCurrentTimeAsString() + ": " + Comment, "ParticipantLog");
	FSFLoggingUtils::Log("Logged Comment: " + Comment);
	if (bAlsoLogToHUD)
	{
		LogToHUD(Comment);
	}
}

void USFGameInstance::LogData(const FString& DependentVariableName, const FString& Value)
{
	if (!GetParticipant())
	{
		return;
	}
	USFCondition* CurrCondition = GetParticipant()->GetCurrentCondition();
	if (!CurrCondition->StoreDependentVariableData(DependentVariableName, Value))
	{
		return;
	}
	LogComment("Recorded " + DependentVariableName + ": " + Value);

	//the data is stored in the phase long table on SetCondition() or EndStudy()
}

void USFGameInstance::LogTrialData(const FString& DependentVariableName, const TArray<FString>& Values)
{
	if (!GetParticipant())
	{
		return;
	}
	USFCondition* CurrCondition = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition();
	if (!CurrCondition->StoreMultipleTrialDependentVariableData(DependentVariableName, Values))
	{
		return;
	}

	FString Data = "";
	for (const FString& Value : Values)
	{
		Data += (Data.IsEmpty() ? "{" : ",") + Value;
	}
	Data += "}";
	LogComment("Recorded " + DependentVariableName + ": " + Data);
}