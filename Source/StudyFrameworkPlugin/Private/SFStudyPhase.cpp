#include "SFStudyPhase.h"

#include "SFUtils.h"


USFStudyPhase::USFStudyPhase()
{
}

void USFStudyPhase::AddStudyFactor(FSFStudyFactor Factor)
{
	Factors.Add(Factor);
}

void USFStudyPhase::AddMap(const FString Name)
{
	MapNames.Add(Name);
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
	if (TypeOfRepetition != EPhaseRepetitionType::SameOrder)
	{
		FSFUtils::Log(
			"Currently only SameOrder repetitions supported! Please implement in USFStudyPhase::GenerateOrder().", true);
	}
}

void USFStudyPhase::SetSettingsMixing(EMixingSetupOrder MixingType)
{
	TypeOfMixing = MixingType;
}

bool USFStudyPhase::PhaseValid()
{
	if (MapNames.Num() == 0)
	{
		FSFUtils::Log("Phase " + GetName() + " is invalid, since no map is set!", true);
		return false;
	}
	return true;
}

bool USFStudyPhase::GenerateOrder()
{
	const int NumberOfFactors = Factors.Num();

	int NumberOfConditions = MapNames.Num();
	for (int i = 0; i < Factors.Num(); i++)
	{
		NumberOfConditions *= Factors[i].Count;
	}

	Orders.Empty();
	Orders.Reserve(NumberOfConditions); //so we have enough space, it is still empty, however

	//TODO: not randomized yet, so add that!
	TArray<int> Order;
	Order.Init(-1, NumberOfFactors + 1); //invalid entries; +1 for level
	for (int MapIndex = 0; MapIndex < MapNames.Num(); ++MapIndex)
	{
		Order[0] = MapIndex;
		for (int FactorIndex = 0; FactorIndex < NumberOfFactors; ++FactorIndex)
		{
			for (int FactorLevel = 0; FactorLevel < Factors[FactorIndex].Count; ++FactorLevel)
			{
				Order[FactorIndex + 1] = FactorLevel;
				Orders.Add(Order);
			}
		}
		if (NumberOfFactors == 0)
		{
			Orders.Add(Order); //add it anyways if we have a phase without settings and only levels
		}
	}


	//TODO this does not care for the TypeOfRepetition, currently it always does SameOrder
	// Now setup repetitions
	for (int r = 0; r < NumberOfRepetitions - 1; r++)
	{
		for (int i = 0; i < NumberOfConditions; i++)
		{
			Orders.Add(Orders[i]);
		}
	}

	return true;
}

TArray<int> USFStudyPhase::NextCondition()
{
	if (Orders.Num() <= ++CurrentCondtitionIdx)
	{
		// Phase already ran all Setups
		return TArray<int>();
	}

	UpcomingCondition = Orders[CurrentCondtitionIdx];

	// Level ID stored in first Entry of Setup
	UpcomingMapName = MapNames[UpcomingCondition[0]];

	return UpcomingCondition;
}

bool USFStudyPhase::ApplyCondition()
{
	bool bSuc = true;
	//starting at 1 since first factor represents the different levels
	for (int i = 1; i < Factors.Num(); i++)
	{
		bSuc &= Factors[i].Delegate.ExecuteIfBound(UpcomingCondition[i]);
	}

	CurrentCondition = UpcomingCondition;
	UpcomingCondition.Empty();

	return bSuc;
}

FString USFStudyPhase::GetUpcomingLevelName() const
{
	return UpcomingMapName;
}

TArray<UClass*> USFStudyPhase::GetSpawnActorsCpp() const
{
	return SpawnInThisPhaseCpp;
}

TArray<FSFClassOfBlueprintActor> USFStudyPhase::GetSpawnActorsBlueprint() const
{
	return SpawnInThisPhaseBlueprint;
}

TArray<int> USFStudyPhase::GetFactorsLevelCount()
{
	TArray<int> Array;
	for (auto Factor : Factors)
	{
		Array.Add(Factor.Count);
	}
	return Array;
}

TArray<FString> USFStudyPhase::GetOrderStrings()
{
	return TArray<FString>();
}

TArray<int> USFStudyPhase::GetCurrentCondition()
{
	return CurrentCondition;
}

const TArray<FString>& USFStudyPhase::GetMapNames() const
{
	return MapNames;
}

const TArray<FSFStudyFactor>& USFStudyPhase::GetFactors() const
{
	return Factors;
}
