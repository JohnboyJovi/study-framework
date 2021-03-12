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
	 if(TypeOfRepetition!= EPhaseRepetitionType::SameOrder)
	 {
		 FSFUtils::LogStuff("Currently only SameOrder repetitions supported! Please implement in USFStudyPhase::GenerateOrder().", true);
	 }
}

void USFStudyPhase::SetSettingsMixing(EMixingSetupOrder MixingType)
{
    TypeOfMixing = MixingType;
}

bool USFStudyPhase::PhaseValid()
{
	if(LevelNames.Num()==0)
	{
		FSFUtils::LogStuff("Phase " + GetName() + " is invalid, since no level is set!", true);
		return false;
	}
	return true;
}

bool USFStudyPhase::GenerateOrder()
{
    const int NumberOfSettings = Settings.Num();

    int NumberOfConditions = LevelNames.Num();
    for (int i = 0; i < Settings.Num(); i++)
    {
		 NumberOfConditions *= Settings[i].Count;
    }

	 Orders.Empty();
    Orders.Reserve(NumberOfConditions); //so we have enough space, it is still empty, however

	 //TODO: not randomized yet, so add that!
	 TArray<int> Order;
	 Order.Init(-1, NumberOfSettings + 1); //invalid entries; +1 for level
	 for(int LevelIndex=0; LevelIndex<LevelNames.Num(); ++LevelIndex)
	 {
		 Order[0] = LevelIndex;
		 for (int SettingIndex = 0; SettingIndex < NumberOfSettings; ++SettingIndex)
		 {
			 for (int SettingLevel = 0; SettingLevel < Settings[SettingIndex].Count; ++SettingLevel)
			 {
				 Order[SettingIndex + 1] = SettingLevel;
				 Orders.Add(Order);
			 }
		 }
	 	if(NumberOfSettings==0)
	 	{
			Orders.Add(Order); //add it anyways if we have a phase without settings and only levels
	 	}
	 }


	 //TODO this does not care for the TypeOfRepetition, currently it always does SameOrder
    // Now setup repetitions
    for (int r = 0; r < NumberOfRepetitions - 1;  r++)
    {
        for (int i = 0; i < NumberOfConditions; i++)
        {
            Orders.Add(Orders[i]);
        }
    }

    return true;
}

TArray<int> USFStudyPhase::NextSetup()
{
    if (Orders.Num() <= ++CurrentSetupIdx)
    {
        // Phase already ran all Setups
        return TArray<int>();
    }

    UpcomingSetup = Orders[CurrentSetupIdx];

    // Level ID stored in first Entry of Setup
    UpcomingLevelName = LevelNames[UpcomingSetup[0]];

    return UpcomingSetup;
}

bool USFStudyPhase::ApplySettings()
{
    bool bSuc = true;
	 //starting at 1 since first setting represents the different levels
    for (int i = 1; i < Settings.Num(); i++)
    {
        bSuc &= Settings[i].Delegate.ExecuteIfBound(UpcomingSetup[i]);
    }

    CurrentSetup = UpcomingSetup;
    UpcomingSetup.Empty();
    
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

