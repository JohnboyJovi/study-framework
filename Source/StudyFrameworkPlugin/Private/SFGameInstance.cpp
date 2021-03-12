// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGameInstance.h"

#include <string>

#include "SFStudyControllerActor.h"

#include "SFFadeHandler.h"
#include "SFDataGameInstance.h"
#include "SFUtils.h"

#include "EngineUtils.h"            // For Spawning in Actor in each level
#include "IUniversalLogging.h"
#include "SFMasterHUD.h"


// ****************************************************************** // 
// ******* Initialization ******************************************* //
// ****************************************************************** //

void USFGameInstance::Initialize(FString ParticipantID, FString JsonFilePath)
{
    if (bInitialized)
    {
        return;
    }

    // FadeHandler
    FadeHandler = NewObject<USFFadeHandler>(GetTransientPackage(), "SFFadeHandler");
    FadeHandler->AddToRoot();                   // TODO What is that?
    FadeHandler->SetGameInstance(this);
    FadeHandler->SetInitialFadedOut(false);

    // Participant
    Participant = NewObject<USFParticipant>(GetTransientPackage(), FName(*ParticipantID));
    Participant->Initialize(ParticipantID, JsonFilePath, this);

    // TODO Check if necessary
    SpawnInEveryPhaseCpp.Add(ASFStudyControllerActor::StaticClass());

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
        FSFUtils::LogStuff("[USFGameInstance::StartStudy()]: Study already started.", true);
        return false;
    }

    if(!Participant->StartStudy())
    {
		 FSFUtils::LogStuff("[USFGameInstance::StartStudy()]: unable to start study.", true);
		 return false;
    }

    bStudyStarted = true;

    NextSetup();
    return true;
}

void USFGameInstance::EndStudy()
{
    Participant->CommitData();
    Participant->EndStudy();
}




bool USFGameInstance::NextSetup()
{
    // Check if is already fading
    if (FadeHandler->GetIsFading())
    {
        FSFUtils::LogStuff("[USFGameInstance::NextSetup()]: Already Fading between levels", true);
        return false;
    }

    // Commit data at SFLogger
    Participant->CommitData();

    FString NextLevelName = Participant->NextSetup();

    if (NextLevelName.Equals(""))
    {
        UniLog.Log("SFLog", "[USFGameInstance::NextSetup()]: Could not load next setup.");
        return false;
    }

    // Fade to next Level
    FadeHandler->FadeToLevel(NextLevelName);
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
}



// ****************************************************************** // 
// ******* Prepare Study ******************************************** //
// ****************************************************************** //

void USFGameInstance::AddPhase(USFStudyPhase* Phase)
{
    if (bStudyStarted)
    {
        FSFUtils::LogStuff("[USFGameInstance::AddPhase()]: Study already started.", true);
    }

	 //this Rename is needed, to unbind the Phase object from any other object class, so it is not garbage collected on level change
	 Phase->Rename(*Phase->GetName(), GetTransientPackage());

    Participant->AddPhase(Phase);
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

    TArray<FSFClassOfBlueprintActor> SpawnInThisPhaseBlueprint = Participant->GetCurrentPhase()->GetSpawnActorsBlueprint();
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

    FSFUtils::LogStuff("[USFGameInstance::SpawnBlueprintActor()]: Unable to spawn blueprint actor ("
        + Actor.Path + "/" + Actor.ClassName + ") cannot be found!", true);
}

void USFGameInstance::OnLevelLoaded()
{
    Participant->GetCurrentPhase()->ApplySettings();
}



void USFGameInstance::UpdateHUD()
{
  // TODO UPDATE HUD HERE DELETED
    // ASFMasterHUD* MasterHUD = Cast<ASFMasterHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
    // 
    // MasterHUD->SetJsonData(Participant->GetJsonFile());
}


