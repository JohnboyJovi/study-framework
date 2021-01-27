// Fill out your copyright notice in the Description page of Project Settings.


#include "SFParticipant.h"

#include "SFGameInstance.h"


#include <string>

#include "Engine/Engine.h"
#include "SFUtils.h"

USFParticipant::USFParticipant()
{
}

USFParticipant::~USFParticipant()
{
}

void USFParticipant::Initialize(FString IdN, USFGameInstance* GameInstanceN)
{
	Id = IdN;
	GameInstance = GameInstanceN;

	if (FindConfigurationFile())
	{
		LoadConfigurationFile();
	}
	else
	{
		// PhaseSettings = GameInstance->GetPhaseSettings();   // TODO nec?
        // Phases = GameInstance->GetPhases();                 // TODO nec?

        CurrentPhase = Phases[0];

		CreateNewConfigurationFile();
	}

	CreateSetupOrder();

	LogSetupOrder();

}

bool USFParticipant::SaveConfigurationFile()
{
	// https://forums.unrealengine.com/development-discussion/c-gameplay-programming/79298-how-to-access-input-ini-in-run-time-and-modify-it
	// https://docs.unrealengine.com/en-US/Gameplay/SaveGame/index.html
	// https://forums.unrealengine.com/unreal-engine/announcements-and-releases/1745504-a-new-community-hosted-unreal-engine-wiki

    // TODO Create Configuration File

	// Von Jonathan: FFileHelper
	return false;
}

bool USFParticipant::FindConfigurationFile()
{
	return false;
}

bool USFParticipant::LoadConfigurationFile()
{
	return false;
}

bool USFParticipant::CreateNewConfigurationFile()
{
    for (auto Phase : Phases)
    {
        PhaseTotalNumberOfConfigurations[Phase] = 1;
	    for (int i = 0; i < PhaseSettings[Phase].Num(); i++)
	    {
	    	PhaseTotalNumberOfConfigurations[Phase] = PhaseTotalNumberOfConfigurations[Phase] * PhaseSettings[Phase][i].Count;
	    }

        // TODO get in?
        if (PhaseSetupOrder.Find(Phase) == nullptr)
        {
            PhaseSetupOrder.Add(Phase, TArray<TArray<int>>());
        }
        else
        {
            PhaseSetupOrder[Phase].Empty();
        }
        PhaseSetupOrder[Phase].SetNum(PhaseTotalNumberOfConfigurations[Phase]);
        
	    
        if (PhaseAlreadyDone.Find(Phase) == nullptr)
        {
            PhaseAlreadyDone.Add(Phase, TArray<bool>());
        }
        else
        {
            PhaseAlreadyDone[Phase].Empty();
        }
        PhaseAlreadyDone[Phase].SetNum(PhaseTotalNumberOfConfigurations[Phase]);

	    for (int i = 0; i < PhaseTotalNumberOfConfigurations[Phase]; i++)
	    {
	    	PhaseAlreadyDone[Phase][i] = false;
	    }

    }

	SaveConfigurationFile();

	return false;
}

bool USFParticipant::DeleteConfigurationFile()
{
	return false;
}

bool USFParticipant::CreateSetupOrder()
{
    /*
    for (auto Phase : Phases)
    {
	    const int NumberOfConf = PhaseSettings[Phase].Num();

	    // Setting up a counter to be able to select every single combination 
	    TArray<int> Counter;
	    Counter.SetNum(NumberOfConf, false);

	    for (int i = 0; i < NumberOfConf; i++)
	    {
	    	Counter[i] = 1;
            
	    }


	    for (int i = 0; i < PhaseTotalNumberOfConfigurations[Phase]; i++)
	    {
	    	PhaseSetupOrder[Phase][i] = Counter;

	    	for (int j = 0; j < NumberOfConf; j++)
	    	{
	    		if (Counter[j] < PhaseSettings[Phase][j].Count)
	    		{
	    			Counter[j] = Counter[j] + 1;;
	    			break;
	    		}

	    		Counter[j] = 1;
	    	}
	    }

    }
    */
	return false;
}

bool USFParticipant::LogSetupOrder()
{
    for (auto Phase : Phases)
    {
	    FSFUtils::LogStuff("[SFParticipant::LogSetupOrder()]: Logging Setup Order now..", false);
	    for (int i = 0; i < PhaseSetupOrder[Phase].Num(); i++)
	    {
	    	FString Text = "";
	    	for (int j = 0; j < PhaseSettings[Phase].Num(); j++)
	    	{
	    		Text = Text + FString::FromInt(PhaseSetupOrder[Phase][i][j]) + "  ";
	    	}
	    	FSFUtils::LogStuff(Text, false);
	    	FSFUtils::PrintToScreen(Text);

	    }
    }

	return true;
}

TArray<int> USFParticipant::GetNextSetup()
{
    if (bFinished)
    {
        return TArray<int>();
    }

	if (NextSetupIdx >= PhaseTotalNumberOfConfigurations[CurrentPhase])
	{
        FSFUtils::LogStuff("[SFParticipant::GetNextSetup()]: Going to next phase", false);

        CurrentPhaseIdx++;

        if (CurrentPhaseIdx >= Phases.Num())
        {
            FSFUtils::LogStuff("[SFParticipant::GetNextSetup()]: There is no next phase", true);

            bFinished = true;

            return TArray<int>();
        }

        CurrentPhase = Phases[CurrentPhaseIdx];

		
	}

	return PhaseSetupOrder[CurrentPhase][NextSetupIdx++];

}

