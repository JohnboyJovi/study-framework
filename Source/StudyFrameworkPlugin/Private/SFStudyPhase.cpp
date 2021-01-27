#include "SFStudyPhase.h"

#include "SFUtils.h"


USFStudyPhase::USFStudyPhase()
{
}

void USFStudyPhase::AddStudySetting(FSFStudySetting Setting)
{
    Settings.Add(Setting);
}

void USFStudyPhase::AddLevel(const FString Name)
{
    LevelNames.Add(Name);
}

void USFStudyPhase::AddActorForEveryLevelInThisPhaseCpp(UClass* Actor)
{
    SpawnInThisPhaseCpp.Add(Actor);
}

void USFStudyPhase::AddActorForEveryLevelInThisPhaseBlueprint(FSFClassOfBlueprintActor Actor)
{
    SpawnInThisPhaseBlueprint.Add(Actor);
}

void USFStudyPhase::SetRepetitions(int Num, EPhaseRepetitionType Type)
{
    NumberOfRepetitions = Num;
    TypeOfRepetition = Type;
}

void USFStudyPhase::SetSettingsMixing(EMixingSetupOrder MixingType)
{
    TypeOfMixing = MixingType;
}

bool USFStudyPhase::PhaseValid()
{
    return (LevelNames.Num() && Settings.Num());
}

bool USFStudyPhase::GenerateOrder()
{
    const int NumberOfConf = Settings.Num();

    NumberOfSetups = 1;
    for (int i = 0; i < Settings.Num(); i++)
    {
        NumberOfSetups = NumberOfSetups * Settings[i].Count;
    }


    // Setting up a counter to be able to select every single combination 
    TArray<int> Counter;
    Counter.SetNum(NumberOfConf, false);

    for (int i = 0; i < NumberOfConf; i++)
    {
        Counter[i] = 1;
    }

    for (int i = 0; i < NumberOfSetups; i++)
    {
        Order2D[i] = Counter;

        for (int j = 0; j < NumberOfConf; j++)
        {
            if (Counter[j] < Settings[j].Count)
            {
                Counter[j] = Counter[j] + 1;;
                break;
            }

            Counter[j] = 1;
        }
    }

    // Now setup repititions
    for (int r = 0; r < NumberOfRepetitions - 1;  r++)
    {
        for (int i = 0; i < NumberOfSetups; i++)
        {
            Order2D.Add(Order2D[i]);
        }
    }

    // Add the Number of repition at last digit in Array
    for (int r = 0; r < NumberOfRepetitions; r++)
    {
        for (int i = 0; i < NumberOfSetups; i++)
        {
            Order2D[r * NumberOfRepetitions + i].Add(r);
        }
    }

    return true;
}

TArray<int> USFStudyPhase::NextSetup()
{
    UpcomingSetup = Order2D[CurrentSetupIdx + 1];
    return UpcomingSetup;
}

bool USFStudyPhase::ApplySettings()
{
    bool bSuc = true; 
    for (int i = 1; i < Settings.Num(); i++)
    {
        bSuc &= Settings[i].Delegate.ExecuteIfBound(UpcomingSetup[i]);
    }

    CurrentSetup = UpcomingSetup;
    UpcomingSetup.Empty();
    CurrentSetupIdx++;
    
    return bSuc;
}

FString USFStudyPhase::GetUpcomingLevelName() const
{
    return UpcomingLevelName;
}

TArray<UClass*> USFStudyPhase::GetSpawnActorsCpp() const
{
    return SpawnInThisPhaseCpp;
}

TArray<FSFClassOfBlueprintActor> USFStudyPhase::GetSpawnActorsBlueprint() const
{
    return SpawnInThisPhaseBlueprint;
}

TArray<int> USFStudyPhase::GetSetupNumArray()
{
    TArray<int> Array;
    for (auto Entry : Settings)
    {
        Array.Add(Entry.Count);
    }
    return Array;
}

TArray<FString> USFStudyPhase::GetSetupOrderArrayString()
{
    return TArray<FString>();
}

TArray<int> USFStudyPhase::GetCurrentSetup()
{
    TArray<int> Tmp = CurrentSetup;
    return CurrentSetup;
}

