#include "SFStudyPhase.h"

#include "Dom/JsonValue.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "Help/SFUtils.h"


USFStudyPhase::USFStudyPhase()
{
}

USFStudyFactor* USFStudyPhase::AddStudyFactor(FString FactorName, const TArray<FString>& FactorLevels)
{
	USFStudyFactor* Factor = NewObject<USFStudyFactor>(this);
	Factor->FactorName = FactorName;
	Factor->Levels = FactorLevels;
	Factors.Add(Factor);
	return Factor;
}

USFMapFactor* USFStudyPhase::AddMapFactor(const TArray<FString>& FactorLevels)
{
	if (ContainsAMapFactor())
	{
		FSFUtils::Log("Already contains Map Factor, {" + FString::Join(FactorLevels, TEXT(", ")) + "} will be ignored",
		              true);
		return nullptr;
	}
	USFMapFactor* Factor = NewObject<USFMapFactor>(this);
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
		FSFUtils::Log("Phase " + PhaseName + " is invalid, since no map is set!", true);
		return false;
	}
	return true;
}

TArray<USFCondition*> USFStudyPhase::GenerateConditions()
{
	TArray<USFCondition*> Conditions;
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

	for (TArray<int> ConditionIndices : ConditionsIndices)
	{
		USFCondition* Condition = NewObject<USFCondition>();
		Condition->Generate(PhaseName, ConditionIndices, Factors, DependentVariables);
		Condition->SpawnInThisCondition.Append(SpawnInEveryMapOfThisPhase);
		Conditions.Add(Condition);
	}

	auto LatinSqTest = [] (int numberConditions) {
		FSFUtils::Log("Latin Square Test for "+FString::FromInt(numberConditions));
		for(int i=0; i<numberConditions*2; ++i)
		{
			FString Content = "";
			TArray<int> Order = USFStudyFactor::GenerateLatinSquareOrder(i, numberConditions);
			for(int j=0; j<Order.Num(); ++j)
			{
				Content += " "+FString::FromInt(Order[j]);
			}
			FSFUtils::Log(Content);
		}
	};

	LatinSqTest(3);
	LatinSqTest(4);
	LatinSqTest(5);

	//TODO this does not care for the TypeOfRepetition, currently it always does SameOrder
	// Now setup repetitions
	for (int r = 0; r < NumberOfRepetitions - 1; r++)
	{
		for (int i = 0; i < NumberOfConditions; i++)
		{
			Conditions.Add(Conditions[i]);
		}
	}

	return Conditions;
}


TArray<TSubclassOf<AActor>> USFStudyPhase::GetSpawnActors() const
{
	return SpawnInEveryMapOfThisPhase;
}

const TArray<USFStudyFactor*> USFStudyPhase::GetFactors() const
{
	return Factors;
}

TSharedPtr<FJsonObject> USFStudyPhase::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	Json->SetStringField("Name", PhaseName);

	// Factors
	TArray<TSharedPtr<FJsonValue>> FactorsArray;
	for (USFStudyFactor* Factor : Factors)
	{
		if (!Factor)
			continue;
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Factor->GetAsJson());
		FactorsArray.Add(JsonValue);
	}
	Json->SetArrayField("Factors", FactorsArray);

	// DependentVariables
	TArray<TSharedPtr<FJsonValue>> DependentVarsArray;
	for (USFDependentVariable* Var : DependentVariables)
	{
		if (!Var)
			continue;
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Var->GetAsJson());
		DependentVarsArray.Add(JsonValue);
	}
	Json->SetArrayField("Dependent Variables", DependentVarsArray);

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
	for (TSubclassOf<AActor> Class : SpawnInEveryMapOfThisPhase)
	{
		if (!Class)
			continue;
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetStringField("ClassName", Class.Get()->GetName());
		JsonObject->SetStringField("ClassPath", Class.Get()->GetPathName());
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(JsonObject);
		SpawnActorsArray.Add(JsonValue);
	}
	Json->SetArrayField("SpawnInEveryMapOfThisPhase", SpawnActorsArray);

	return Json;
}

void USFStudyPhase::FromJson(TSharedPtr<FJsonObject> Json)
{
	PhaseName = Json->GetStringField("Name");

	// Factors
	TArray<TSharedPtr<FJsonValue>> FactorsArray = Json->GetArrayField("Factors");
	for (auto JsonFactor : FactorsArray)
	{
		TSharedPtr<FJsonObject> FactorObj = JsonFactor->AsObject();
		USFStudyFactor* Factor = nullptr;
		if (FactorObj->HasField("MapFactor") && FactorObj->GetBoolField("MapFactor"))
		{
			Factor = NewObject<USFMapFactor>(this);
		}
		else
		{
			Factor = NewObject<USFStudyFactor>(this);
		}
		Factor->FromJson(FactorObj);
		Factors.Add(Factor);
	}

	// DependentVariables
	TArray<TSharedPtr<FJsonValue>> DependentVarsArray = Json->GetArrayField("Dependent Variables");;
	for (auto Var : DependentVarsArray)
	{
		USFDependentVariable* DependentVariable = NewObject<USFDependentVariable>(this);
		DependentVariable->FromJson(Var->AsObject());
		DependentVariables.Add(DependentVariable);
	}

	// NumberOfRepetitions
	NumberOfRepetitions = Json->GetNumberField("Number Of Repetitions");

	// TypeOfRepetition
	FString RepetitionTypeStr = Json->GetStringField("TypeOfRepetition");
	if (RepetitionTypeStr == "SameOrder")
	{
		TypeOfRepetition = EPhaseRepetitionType::SameOrder;
	}
	else if (RepetitionTypeStr == "DifferentOrder")
	{
		TypeOfRepetition = EPhaseRepetitionType::DifferentOrder;
	}
	else if (RepetitionTypeStr == "FullyRandom")
	{
		TypeOfRepetition = EPhaseRepetitionType::FullyRandom;
	}
	else
	{
		FSFUtils::Log("[USFStudyPhase::FromJson] unknown TypeOfRepetition: " + RepetitionTypeStr, true);
	}

	// SpawnInEveryMapOfThisPhase
	TArray<TSharedPtr<FJsonValue>> SpawnActorsArray = Json->GetArrayField("SpawnInEveryMapOfThisPhase");;
	for (auto Class : SpawnActorsArray)
	{
		const FString Name = Class->AsObject()->GetStringField("ClassName");
		const FString Path = Class->AsObject()->GetStringField("ClassPath");
		const FString FullName = Path + "/" + Name;
		UClass* ClassToSpawn = FindObject<UClass>(ANY_PACKAGE, *FullName);
		if (!ClassToSpawn)
		{
			FSFUtils::Log("[USFStudyPhase::FromJson] class does not exist: " + Path + "/" + Name, true);
			continue;
		}
		SpawnInEveryMapOfThisPhase.Add(ClassToSpawn->GetClass());
	}
}

bool USFStudyPhase::ContainsNullptrInArrays()
{
	for (USFStudyFactor* Factor : Factors)
	{
		if (Factor == nullptr)
		{
			return true;
		}
	}
	for (USFDependentVariable* Var : DependentVariables)
	{
		if (Var == nullptr)
		{
			return true;
		}
	}
	return false;
}

bool USFStudyPhase::ContainsAMapFactor() const
{
	return GetMapFactor() != nullptr;
}

USFMapFactor* USFStudyPhase::GetMapFactor() const
{
	int MapIdx = GetMapFactorIndex();
	if (MapIdx == -1)
	{
		return nullptr;
	}
	return Cast<USFMapFactor>(Factors[MapIdx]);
}

int USFStudyPhase::GetMapFactorIndex() const
{
	for (int i = 0; i < Factors.Num(); ++i)
	{
		if (Factors[i]->IsA(USFMapFactor::StaticClass()))
		{
			return i;
		}
	}
	return -1;
}

void USFStudyPhase::CreateAllConditionsRecursively(int Index, TArray<int> OrderPart, TArray<TArray<int>>& OrdersIndices) const
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
