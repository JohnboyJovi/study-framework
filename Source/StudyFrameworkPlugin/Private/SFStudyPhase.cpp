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
	if (MapFactor == nullptr || MapFactor->Levels.Num() == 0)
	{
		FSFUtils::OpenMessageBox("Phase " + PhaseName + " is invalid, since no map is set!", true);
		return false;
	}

	int NrEnBlockFactors = 0;
	int NrRandomFactors = 0;

	for (USFStudyFactor* Factor : Factors)
	{
		// Not Implemented YET errors
		if (Factor->Type == EFactorType::Between)
		{
			FSFUtils::OpenMessageBox(
				"[USFStudyPhase::PhaseValid] Between factors are not yet implemented! Used for factor " + Factor->FactorName
				+ " in phase " + PhaseName, true);
			return false;
		}

		if (TypeOfRepetition != EPhaseRepetitionType::SameOrder && NumberOfRepetitions > 1)
		{
			FSFUtils::OpenMessageBox(
				"[USFStudyPhase::PhaseValid] Only same order repetition is implemented but something else was requested in phase "
				+ PhaseName, true);
			return false;
		}


		// Other (permanent) problems
		if (Factor->Levels.Num() == 0)
		{
			FSFUtils::OpenMessageBox(
				"[USFStudyPhase::PhaseValid] No level was set for factor " + Factor->FactorName + " in phase " + PhaseName,
				true);
			return false;
		}

		if (Factor->FactorName == "")
		{
			FSFUtils::OpenMessageBox("[USFStudyPhase::PhaseValid] No name set for a factor in phase " + PhaseName, true);
			return false;
		}

		NrEnBlockFactors += (Factor->MixingOrder == EFactorMixingOrder::EnBlock ? 1 : 0);
		NrRandomFactors += (Factor->MixingOrder == EFactorMixingOrder::RandomOrder ? 1 : 0);

		if (NrEnBlockFactors > 1)
		{
			//what to actually do when more than one factor wants that???
			FSFUtils::OpenMessageBox(
				"[USFStudyPhase::PhaseValid] " + Factor->FactorName + " in phase " + PhaseName +
				" is already the second enBlock factor, how should that work? If you know, implement ;-)", true);
			return false;
		}
	}

	return true;
}


TArray<USFCondition*> USFStudyPhase::GenerateConditions(int ParticipantNr)
{
	// first restructure factors, such that:
	// the map factor is the first one
	// a potential enBlock factor is the last one
	TArray<USFStudyFactor*> SortedFactors;
	bool bHasEnBlock = SortFactors(SortedFactors);

	TArray<USFCondition*> Conditions;
	int NumberOfConditions = 1;
	for (int i = 0; i < SortedFactors.Num(); i++)
	{
		NumberOfConditions *= SortedFactors[i]->Levels.Num();
	}


	// ****************************
	// Generate Condition Indices
	// ****************************

	Conditions.Empty();
	Conditions.Reserve(NumberOfRepetitions * NumberOfConditions); //so we have enough space, it is still empty, however

	//create an array holding for each condition an array of each factors' level index
	TArray<TArray<int>> ConditionsIndices;
	ConditionsIndices.Reserve(NumberOfConditions);
	CreateAllConditionsRecursively(0, {}, SortedFactors, ConditionsIndices);


	// ****************************
	//          Randomize
	// ****************************


	int NrLatinSqConditions = ConditionsIndices.Num();
	int enBlockConditions = 1;
	if (bHasEnBlock)
	{
		//that last enBlock factor is not shuffled since it is en block already by construction
		//never the less, we still have to change the order within the block, that's what we use LatinSquareRndReOrderEnBlock for
		enBlockConditions = SortedFactors.Last()->Levels.Num();
		NrLatinSqConditions /= enBlockConditions;
	}
	const TArray<int> LatinSquareRndReOrder = USFStudyFactor::GenerateLatinSquareOrder(
		ParticipantNr, NrLatinSqConditions);
	const TArray<int> LatinSquareRndReOrderEnBlock = USFStudyFactor::GenerateLatinSquareOrder(
		ParticipantNr, enBlockConditions);
	TArray<TArray<int>> ConditionsIndicesCopy = ConditionsIndices;
	ConditionsIndices.Empty();
	for (int i = 0; i < enBlockConditions; ++i)
	{
		// if we have enBlockConditions>1 we need to copy and shuffle the whole enBlock Block, otherwise the i loop is trivially run once only
		for (int j = 0; j < LatinSquareRndReOrder.Num(); ++j)
		{
			ConditionsIndices.Add(
				ConditionsIndicesCopy[enBlockConditions * LatinSquareRndReOrder[j] + LatinSquareRndReOrderEnBlock[i]]);
		}
	}


	// ****************************
	//   Create actual conditons
	// ****************************

	for (TArray<int> ConditionIndices : ConditionsIndices)
	{
		USFCondition* Condition = NewObject<USFCondition>();
		Condition->Generate(PhaseName, ConditionIndices, SortedFactors, DependentVariables);
		Condition->SpawnInThisCondition.Append(SpawnInEveryMapOfThisPhase);
		Conditions.Add(Condition);
	}


	// ****************************
	//        Add repetitions
	// ****************************

	//TODO this does not care for the TypeOfRepetition, currently it always does SameOrder
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

bool USFStudyPhase::SortFactors(TArray<USFStudyFactor*>& SortedFactors) const
{
	SortedFactors.Add(GetMapFactor());
	USFStudyFactor* EnBlockFactor = nullptr;
	bool bHasEnBlock = false;
	for (USFStudyFactor* Factor : Factors)
	{
		if (Factor->IsA(USFMapFactor::StaticClass()))
		{
			continue; //already added it above
		}
		if (Factor->MixingOrder != EFactorMixingOrder::EnBlock)
		{
			SortedFactors.Add(Factor);
		}
		else if (Factor->MixingOrder == EFactorMixingOrder::EnBlock)
		{
			EnBlockFactor = Factor;
		}
	}
	if (EnBlockFactor)
	{
		bHasEnBlock = true;
		SortedFactors.Add(EnBlockFactor);
	}
	return bHasEnBlock;
}

void USFStudyPhase::CreateAllConditionsRecursively(int Index, TArray<int> OrderPart,
                                                   TArray<USFStudyFactor*>& SortedFactors,
                                                   TArray<TArray<int>>& OrdersIndices) const
{
	if (Index >= SortedFactors.Num())
	{
		OrdersIndices.Add(OrderPart);
		return;
	}

	for (int i = 0; i < SortedFactors[Index]->Levels.Num(); ++i)
	{
		TArray<int> Order = OrderPart;
		Order.Add(i);
		CreateAllConditionsRecursively(Index + 1, Order, SortedFactors, OrdersIndices);
	}
}
