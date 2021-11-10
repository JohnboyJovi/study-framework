#include "SFStudyPhase.h"

#include "SFUtils.h"


USFStudyPhase::USFStudyPhase()
{
}

USFStudyFactor* USFStudyPhase::AddStudyFactor(FString FactorName, TArray<FString> FactorLevels)
{
	USFStudyFactor* Factor = NewObject<USFStudyFactor>(this, FName(FactorName));
	Factor->FactorName = FactorName;
	Factor->Levels = FactorLevels;
	Factors.Add(Factor);
	return Factor;
}

void USFStudyPhase::AddMap(const FString Name)
{
	MapNames.Add(Name);
}

void USFStudyPhase::AddDependentVariable(FString Name)
{
	USFDependentVariable* Variable = NewObject<USFDependentVariable>(this, FName(Name));
	Variable->Name = Name;
	DependentVariables.Add(Variable);
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
		NumberOfConditions *= Factors[i]->Levels.Num();
	}

	Orders.Empty();
	Orders.Reserve(NumberOfRepetitions * NumberOfConditions); //so we have enough space, it is still empty, however

	//TODO: not randomized yet, so add that!
	TArray<TArray<int>> OrdersIndices;
	OrdersIndices.Reserve(NumberOfConditions);
	CreateAllOrdersRecursively(0, {}, OrdersIndices);

	TArray<FString> Order;
	Order.Reserve(NumberOfFactors + 1); //Map is first in the order
	for (TArray<int> OrderIndices : OrdersIndices)
	{
		Order.Empty();
		Order.Add(MapNames[OrderIndices[0]]);
		for (int i = 1; i < OrderIndices.Num(); ++i)
		{
			Order.Add(Factors[i - 1]->Levels[OrderIndices[i]]);
		}
		Orders.Add(Order);
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

TArray<FString> USFStudyPhase::NextCondition()
{
	if (Orders.Num() <= ++CurrentCondtitionIdx)
	{
		// Phase already ran all Setups
		return TArray<FString>();
	}

	UpcomingCondition = Orders[CurrentCondtitionIdx];

	// Level ID stored in first Entry of Setup
	UpcomingMapName = UpcomingCondition[0];

	return UpcomingCondition;
}

bool USFStudyPhase::ApplyCondition()
{
	bool bSuc = true;

	//TODO: should we trigger something here?
	//starting at 1 since first factor represents the different levels
	/*for (int i = 1; i < Factors.Num(); i++)
	{
		bSuc &= Factors[i].Delegate.ExecuteIfBound(UpcomingCondition[i]);
	}*/

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
		Array.Add(Factor->Levels.Num());
	}
	return Array;
}

TArray<FString> USFStudyPhase::GetOrderStrings()
{
	return TArray<FString>();
}

TArray<FString> USFStudyPhase::GetCurrentCondition()
{
	return CurrentCondition;
}

const TArray<FString>& USFStudyPhase::GetMapNames() const
{
	return MapNames;
}

const TArray<USFStudyFactor*> USFStudyPhase::GetFactors() const
{
	return Factors;
}

void USFStudyPhase::CreateAllOrdersRecursively(int Index, TArray<int> OrderPart, TArray<TArray<int>>& OrdersIndices)
{
	if (Index > Factors.Num())
	{
		OrdersIndices.Add(OrderPart);
		return;
	}

	int NumOfFactors = MapNames.Num();
	if (Index != 0)
	{
		//iterate through factors, not the maps which is always the first in the orderIndices
		NumOfFactors = Factors[Index - 1]->Levels.Num();
	}

	for (int i = 0; i < NumOfFactors; ++i)
	{
		TArray<int> Order = OrderPart;
		Order.Add(i);
		CreateAllOrdersRecursively(Index + 1, Order, OrdersIndices);
	}
}
