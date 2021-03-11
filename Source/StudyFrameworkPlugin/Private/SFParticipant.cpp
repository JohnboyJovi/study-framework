// Fill out your copyright notice in the Description page of Project Settings.


#include "SFParticipant.h"

#include "SFGameInstance.h"

#include "SFLogger.h"

#include <string>


#include "IUniversalLogging.h"
#include "Engine/Engine.h"
#include "SFUtils.h"

USFParticipant::USFParticipant()
{
}

USFParticipant::~USFParticipant()
{
}

void USFParticipant::SaveDataArray(FString Where, TArray<FString> Data)
{
    TArray<int> CurrentSetup = CurrentPhase->GetCurrentSetup();

    FString Setup = FSFUtils::SetupToString(CurrentSetup);

    Logger->SaveDataArray(Where, Data, CurrentPhaseIdx, Setup);
}

void USFParticipant::AddPhase(USFStudyPhase* PhaseNew)
{
    Phases.Add(PhaseNew);
}

bool USFParticipant::CheckPhases()
{
    for (auto EntryPhase : Phases)
    {
        if (!EntryPhase->PhaseValid())
        {
            return false;
        }
    }

    return true;
}

bool USFParticipant::Initialize(FString IdNew, FString JsonFilePath, USFGameInstance* GameInstanceNew, FString LogName, FString SaveDataLogName)
{
    ParticipantID = IdNew;

    GameInstance = GameInstanceNew;

    // TODO initialize Logger!!!
    Logger = NewObject<USFLogger>();
    Logger->Initialize(this, JsonFilePath, LogName, SaveDataLogName);

    return true;
}

void USFParticipant::GenerateInitialJsonFile()
{
    // Create initial Json File
    /*
     + Phases (object)
     +     Num (Int)
     +         5 (Value)
     +     1 (object)
     +         Setup (Array)
     +             2_2_2 (Value)
     +         Order (FString Array)
     +             1_1,1_2,2_1,2_2 (Value)

     + "Data": (object)
     +     "Phase_1": (object)
     +         "1_1": (object)
     -             "NameOfData": (Array)
     >                  1,2,3,4,5 (Value)
    */

    MainJsonObject = MakeShared<FJsonObject>();

    TSharedPtr<FJsonObject> JsonPhases = MakeShared<FJsonObject>();
    TSharedPtr<FJsonObject> JsonData = MakeShared<FJsonObject>();

    const int NumPhases = Phases.Num();

    JsonPhases->SetNumberField("NumberOfPhases", NumPhases);

    TArray<TSharedPtr<FJsonObject>> JsonEachPhase;

    for (int i = 0; i < NumPhases; i++)
    {
        TSharedPtr<FJsonObject> JsonTmpPhases = MakeShared<FJsonObject>();
        TSharedPtr<FJsonObject> JsonTmpData = MakeShared<FJsonObject>();

        // Setup
        TArray<int> SetupInt = Phases[i]->GetSetupNumArray();
        TArray<TSharedPtr<FJsonValue>> Setup;
        for (auto Entry : SetupInt)
        {
            TSharedPtr<FJsonValueNumber> Tmp = MakeShared<FJsonValueNumber>(Entry);
            Setup.Add(Tmp);
        }
        JsonTmpPhases->SetArrayField("Setup", Setup);

        // Order
        TArray<FString> OrderFString = Phases[i]->GetSetupOrderArrayString();
        TArray<TSharedPtr<FJsonValue>> Order;
        for (auto Entry : OrderFString)
        {
            TSharedPtr<FJsonValueString> Tmp = MakeShared<FJsonValueString>(Entry);
            Order.Add(Tmp);
        }
        JsonTmpPhases->SetArrayField("Order", Setup);

        // Add Phase
        JsonPhases->SetObjectField(FString::FromInt(i), JsonTmpPhases);


        // Data Part
        for (auto Entry : OrderFString)
        {
            TSharedPtr<FJsonObject> Tmp = MakeShared<FJsonObject>();
            JsonTmpData->SetObjectField(Entry, Tmp);
        }

        JsonData->SetObjectField(FString::FromInt(i), JsonTmpData);

    }

    MainJsonObject->SetObjectField("Phases", JsonPhases);
    MainJsonObject->SetObjectField("Data", JsonData);

    FString JsonAsString = FSFUtils::JsonToString(MainJsonObject);

    FSFUtils::LogStuff(JsonAsString, false);
}

bool USFParticipant::FindJsonFile()
{
    return false;
}

bool USFParticipant::LoadJsonFile()
{
    // TODO set CurrentPhase 
    // TODO set CurrentPhaseIdx 
    return false;
}

bool USFParticipant::StartStudy()
{
    // If reload an already existing study?
    if (FindJsonFile())
    {
        UniLog.Log("SFLog", "[USFParticipant::StartStudy()]: Json File found. Loading it now..");
        return LoadJsonFile();
    }
    else
    {
        
    }

    if (!CheckPhases())
    {
        UniLog.Log("SFLog", "[USFParticipant::StartStudy()]: Not al Phases valid");
        return false;
    }


    // Setup order
    for (auto EntryPhase : Phases)
    {
        EntryPhase->GenerateOrder();
    }

    UniLog.Log("SFLog", "[USFParticipant::StartStudy()]: Generated Phases for " 
        + FString::FromInt(Phases.Num()) + " phases");

    // Create initial Json file
    GenerateInitialJsonFile();

    // And save it
    Logger->SaveJsonFile(MainJsonObject);

    // Set first phase
    CurrentPhase = Phases[0];
    CurrentPhaseIdx = 0;

    return true;
}

FString USFParticipant::NextSetup()     // TODO can maybe be made a schöner function with different if phase finished logic
{
    // Get next Setup
    UpcomingSetup = CurrentPhase->NextSetup();

    if (UpcomingSetup.Num() == 0)
    {
        if (CurrentPhaseIdx >= (Phases.Num() - 1)) // So there is no next phase
        {
            UniLog.Log("SFLog", "[USFParticipant::NextSetup()]: No All setups already ran.");

            GameInstance->EndStudy();

            return "";
        }
        else
        {
            CurrentPhase = Phases[++CurrentPhaseIdx];

            UpcomingSetup = CurrentPhase->NextSetup();
        }
    }

    const FString LevelName = CurrentPhase->GetUpcomingLevelName();

    return LevelName;
}

void USFParticipant::EndStudy()
{

}

void USFParticipant::LogData(FString Data)
{
    Logger->LogData(Data);
}

void USFParticipant::CommitData()
{
    Logger->CommitData();
}

TSharedPtr<FJsonObject> USFParticipant::GetJsonFile()
{
    return MainJsonObject;
}

USFStudyPhase* USFParticipant::GetCurrentPhase()
{
    return CurrentPhase;
}

int USFParticipant::GetCurrentPhaseIdx()
{
    return CurrentPhaseIdx;
}

FString USFParticipant::GetID()
{
    return ParticipantID;
}


