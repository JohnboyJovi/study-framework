#include "SFStudyPhase.h"

#include "Dom/JsonValue.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "Help/SFUtils.h"


USFStudyPhase::USFStudyPhase()
{
}

USFStudyFactor* USFStudyPhase::AddStudyFactor(FString FactorName, const TArray<FString>& FactorLevels)
{
	USFStudyFactor* Factor = NewObject<USFStudyFactor>(this, FName(FactorName));
	Factor->FactorName = FactorName;
	Factor->Levels = FactorLevels;
	Factors.Add(Factor);
	return Factor;
}

USFMapFactor* USFStudyPhase::AddMapFactor(const TArray<FString>& FactorLevels)
{
	if(ContainsAMapFactor())
	{
		FSFUtils::Log("Already contains Map Factor, {"+FString::Join(FactorLevels, TEXT(", "))+"} will be ignored", true);
		return nullptr;
	}
	USFMapFactor* Factor = NewObject<USFMapFactor>(this, "Map");
	Factor->FactorName = "Map";
	Factor->Levels = FactorLevels;
	Factors.Add(Factor);
	return Factor;
}

void USFStudyPhase::AddDependentVariable(FString Name)
{
	USFDependentVariable* Variable = NewObject<USFDependentVariable>(this, FName(Name));
	Variable->Name = Name;
	DependentVariables.Add(Variable);
}

void USFStudyPhase::AddActorForEveryMapInThisPhase(TSubclassOf<AActor> Actor)
{
	SpawnInEveryMapOfThisPhase.Add(Actor);
}

void USFStudyPhase::AddBlueprintActorForEveryMapInThisPhase(const FString& BlueprintPath, const FString& BlueprintName)
{
	SpawnInEveryMapOfThisPhase.Add(FSFUtils::GetBlueprintClass(BlueprintName, BlueprintPath));
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

bool USFStudyPhase::PhaseValid() const
{
	USFMapFactor* MapFactor = GetMapFactor();
	if (MapFactor==nullptr || MapFactor->Levels.Num()==0)
	{
		FSFUtils::Log("Phase " + GetName() + " is invalid, since no map is set!", true);
		return false;
	}
	return true;
}

bool USFStudyPhase::GenerateConditions()
{
	const int NumberOfFactors = Factors.Num();

	int NumberOfConditions = 1;
	for (int i = 0; i < Factors.Num(); i++)
	{
		NumberOfConditions *= Factors[i]->Levels.Num();
	}

	Conditions.Empty();
	Conditions.Reserve(NumberOfRepetitions * NumberOfConditions); //so we have enough space, it is still empty, however

	//TODO: not randomized yet, so add that!
	TArray<TArray<int>> ConditionsIndices;
	ConditionsIndices.Reserve(NumberOfConditions);
	CreateAllConditionsRecursively(0, {}, ConditionsIndices);

	TArray<FString> Condition;
	Condition.Reserve(NumberOfFactors);
	for (TArray<int> ConditionIndices : ConditionsIndices)
	{
		Condition.Empty();
		for (int i = 0; i < ConditionIndices.Num(); ++i)
		{
			Condition.Add(Factors[i]->Levels[ConditionIndices[i]]);
		}
		Conditions.Add(Condition);
	}


	//TODO this does not care for the TypeOfRepetition, currently it always does SameOrder
	// Now setup repetitions
	for (int r = 0; r < NumberOfRepetitions - 1; r++)
	{
		for (int i = 0; i < NumberOfConditions; i++)
		{
			Conditions.Add(Conditions[i]);
		}
	}

	return true;
}

TArray<FString> USFStudyPhase::NextCondition()
{
	if (Conditions.Num() <= ++CurrentCondtitionIdx)
	{
		// Phase already ran all Setups
		return TArray<FString>();
	}

	UpcomingCondition = Conditions[CurrentCondtitionIdx];

	const int MapIdx = GetMapFactorIndex();
	UpcomingMapName = UpcomingCondition[MapIdx];

	return UpcomingCondition;
}

bool USFStudyPhase::ApplyCondition()
{
	//TODO: should we trigger something here?
	//starting at 1 since first factor represents the different levels
	/*for (int i = 1; i < Factors.Num(); i++)
	{
		bSuc &= Factors[i].Delegate.ExecuteIfBound(UpcomingCondition[i]);
	}*/

	CurrentCondition = UpcomingCondition;
	UpcomingCondition.Empty();

	return true;
}

FString USFStudyPhase::GetUpcomingLevelName() const
{
	return UpcomingMapName;
}

TArray<TSubclassOf<AActor>> USFStudyPhase::GetSpawnActors() const
{
	return SpawnInEveryMapOfThisPhase;
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

TArray<FString> USFStudyPhase::GetCurrentCondition() const
{
	return CurrentCondition;
}

const TArray<USFStudyFactor*> USFStudyPhase::GetFactors() const
{
	return Factors;
}

TSharedPtr<FJsonObject> USFStudyPhase::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	Json->SetStringField("Name", GetName());

	// Factors
	TArray<TSharedPtr<FJsonValue>> FactorsArray;
	for(USFStudyFactor* Factor : Factors){
		TSharedRef< FJsonValueObject > JsonValue = MakeShared<FJsonValueObject>(Factor->GetAsJson());
		FactorsArray.Add(JsonValue);
	}
	Json->SetArrayField("Factors",FactorsArray);

	// DependentVariables
	TArray<TSharedPtr<FJsonValue>> DependentVarsArray;
	for(USFDependentVariable* Var : DependentVariables){
		TSharedRef< FJsonValueObject > JsonValue = MakeShared<FJsonValueObject>(Var->GetAsJson());
		DependentVarsArray.Add(JsonValue);
	}
	Json->SetArrayField("Dependent Variables",DependentVarsArray);

	// NumberOfRepetitions
	Json->SetNumberField("Number Of Repetitions", NumberOfRepetitions);

	// TypeOfRepetition
	switch (TypeOfRepetition)
	{
	case EPhaseRepetitionType::SameOrder:
		Json->SetStringField("TypeOfRepetition", "SameOrder");
		break;
	case EPhaseRepetitionType::DifferentOrder:
		Json->SetStringField("TypeOfRepetition", "DifferentOrder");
		break;
		case EPhaseRepetitionType::FullyRandom:
		Json->SetStringField("TypeOfRepetition", "FullyRandom");
		break;
	default:
		FSFUtils::Log("[USFStudyPhase::GetAsJson] unknown TypeOfRepetition!", true);
	}

	// SpawnInEveryMapOfThisPhase
	TArray<TSharedPtr<FJsonValue>> SpawnActorsArray;
	for(TSubclassOf<AActor> Class : SpawnInEveryMapOfThisPhase){
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetStringField("ClassName", Class.Get()->GetName());
		JsonObject->SetStringField("ClassPath", Class.Get()->GetPathName());
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(JsonObject);
		SpawnActorsArray.Add(JsonValue);
	}
	Json->SetArrayField("SpawnInEveryMapOfThisPhase",SpawnActorsArray);	
	
	return  Json;
}

bool USFStudyPhase::ContainsAMapFactor() const
{
	return GetMapFactor()!=nullptr;
}

USFMapFactor* USFStudyPhase::GetMapFactor() const
{
	int MapIdx = GetMapFactorIndex();
	if( MapIdx==-1)
	{
		return nullptr;
	}
	return Cast<USFMapFactor>(Factors[MapIdx]);
}

int USFStudyPhase::GetMapFactorIndex() const
{
	for(int i=0; i<Factors.Num(); ++i)
	{
		if(Factors[i]->IsA(USFMapFactor::StaticClass()))
		{
			return i;
		}
	}
	return -1;
}

void USFStudyPhase::CreateAllConditionsRecursively(int Index, TArray<int> OrderPart, TArray<TArray<int>>& OrdersIndices)
{
	if (Index >= Factors.Num())
	{
		OrdersIndices.Add(OrderPart);
		return;
	}

	for (int i = 0; i < Factors[Index]->Levels.Num(); ++i)
	{
		TArray<int> Order = OrderPart;
		Order.Add(i);
		CreateAllConditionsRecursively(Index + 1, Order, OrdersIndices);
	}
}
