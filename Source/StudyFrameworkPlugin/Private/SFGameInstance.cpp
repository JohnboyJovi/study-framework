// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGameInstance.h"

#include <string>

#include "SFStudyControllerActor.h"

#include "SFFadeHandler.h"
#include "SFDataGameInstance.h"
#include "SFUtils.h"

#include "EngineUtils.h"            // For Spawning in Actor in each level


bool USFGameInstance::NextSetup()
{
    // Check if is already fading
    if (FadeHandler->GetIsFading())
    {
        FSFUtils::LogStuff("[USFGameInstance::NextSetup()]: Already Fading between levels", true);
        return false;
    }

    // Commit data at SFLogger
    Logger->CommitData();
    
    // Get next Setup
    TArray<int> NextSetup = CurrentPhase->NextSetup();
    if (NextSetup.Num() == 0)
    {
        if (CurrentPhaseIdx >= (Phases.Num() - 1)) // So there is no next phase
        {
            FSFUtils::LogStuff("[USFGameInstance::NextSetup()]: No All setups already ran.", false);
            EndStudy();
            return false;
        }
        else
        {
            CurrentPhase = Phases[++CurrentPhaseIdx];
        }
    }
    
    const FString LevelName = CurrentPhase->GetUpcomingLevelName();

    // Fade to next Level
    FadeHandler->FadeToLevel(LevelName);
    return true;
}

void USFGameInstance::OnLevelLoaded()
{
    CurrentPhase->ApplySettings();
}


bool USFGameInstance::IsStarted() const
{
    return bStudyStarted;
}

void USFGameInstance::AddPhase(USFStudyPhase* Phase)
{
    if (bStudyStarted)
    {
        FSFUtils::LogStuff("[USFGameInstance::AddPhase()]: Study already started.", true);
    }
    
    Phases.Add(Phase);
}

void USFGameInstance::AddActorForEveryLevelInEveryPhaseCpp(UClass* Actor)
{
    SpawnInEveryPhaseCpp.Add(Actor);
}

void USFGameInstance::AddActorForEveryLevelInEveryPhaseBlueprint(const FSFClassOfBlueprintActor Actor)
{
    SpawnInEveryPhaseBlueprint.Add(Actor);
}

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
    TArray<UClass*> SpawnInThisPhaseCpp = CurrentPhase->GetSpawnActorsCpp();
    for (auto EntryC : SpawnInThisPhaseCpp)
    {
        GetWorld()->SpawnActor(EntryC);
    }

    TArray<FSFClassOfBlueprintActor> SpawnInThisPhaseBlueprint = CurrentPhase->GetSpawnActorsBlueprint();
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

    FSFUtils::LogStuff("[USFFadeHandler::SpawnBlueprintActor()]: Unable to spawn blueprint actor ("
        + Actor.Path + "/" + Actor.ClassName + ") cannot be found!", true);
}

bool USFGameInstance::StartStudy()
{
	if (bStudyStarted)
	{
		FSFUtils::LogStuff("[USFGameInstance::StartStudy()]: Study already started.", true);
        return false;
	}
    
	Participant = NewObject<USFParticipant>();
    
	Participant->Initialize("Test", this);
    
	bStudyStarted = true;
    
    // Check if all Phases are valid
    int Valid = true;
    for (auto Phase : Phases)
    {
        if(Phase->PhaseValid())
        {
            Phase->GenerateOrder();
        }
        else
        {
            Valid = false;
        }
    }
    
    if (!Valid)
    {
        FSFUtils::LogStuff("[USFGameInstance::StartStudy()]: Not all phases are valid. Cannot start study.", true);
        return false;
    }
    
    FString Log = "[USFGameInstance::StartStudy()]: Phases: ";
    FSFUtils::LogStuff("[USFGameInstance::StartStudy()]: There are " + FString::FromInt(Phases.Num()) + "phases: ", false);

    Logger->Initialize(Phases);

	NextSetup();
    return true;
}

void USFGameInstance::EndStudy()
{

}


// Data Stuff

void USFGameInstance::Initialize()
{
    if (bInitialized)
    {
        return;
    }

    FadeHandler = NewObject<USFFadeHandler>();
    FadeHandler->AddToRoot();                   // TODO What is that?
    FadeHandler->SetGameInstance(this);
    FadeHandler->SetInitialFadedOut(false);

    // TODO Check if necessary
    SpawnInEveryPhaseCpp.Add(ASFStudyControllerActor::StaticClass());

	bInitialized = true;
}

bool USFGameInstance::IsInitialized() const
{
    return bInitialized;
}


bool USFGameInstance::HasData() const
{
    // return (Participant != nullptr && Phases.Num());
    return true;
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

void USFGameInstance::SaveData(const FString Where, FString Data)
{
    TArray<int> CurrentSetup = CurrentPhase->GetCurrentSetup();

    FString Setup = FSFUtils::SetupToString(CurrentSetup);

    Logger->SaveData(Where, Data, CurrentPhaseIdx, Setup);
}

void USFGameInstance::LogData(const FString String)
{
    Logger->LogData(String);
}

void USFGameInstance::CommitData()
{
    Logger->CommitData();
}


