#include "SFStudyPhase.h"

#include "Dom/JsonValue.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "Help/SFUtils.h"
#include "Logging/SFLoggingUtils.h"


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
		FSFLoggingUtils::Log("Already contains Map Factor, {" + FString::Join(FactorLevels, TEXT(", ")) + "} will be ignored",
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

void USFStudyPhase::SetRepetitions(int Num, EPhaseRepetitionType Type)
{
	NumberOfRepetitions = Num;
	TypeOfRepetition = Type;
	if (TypeOfRepetition != EPhaseRepetitionType::SameOrder)
	{
		FSFLoggingUtils::Log(
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
	int NrNonCombinedFactors = 0;

	for (USFStudyFactor* Factor : Factors)
	{

		if (Factor->Levels.Num() == 0)
		{
			FSFUtils::OpenMessageBox(
				"[USFStudyPhase::PhaseValid] No level was set for factor " + Factor->FactorName + " in phase " + PhaseName,
				true);
			return false;
		}

		TSet<FString> ContainedLevels;
		for(const FString& Level : Factor->Levels) {
			if(ContainedLevels.Contains(Level)) {
				FSFUtils::OpenMessageBox(
				"[USFStudyPhase::PhaseValid] Two levels with identical name ("+Level+")  found for factor " + Factor->FactorName + " in phase " + PhaseName,
				true);
				return false;
			}
			ContainedLevels.Add(Level);
		}

		if (Factor->FactorName == "")
		{
			FSFUtils::OpenMessageBox("[USFStudyPhase::PhaseValid] No name set for a factor in phase " + PhaseName, true);
			return false;
		}

		NrEnBlockFactors += (Factor->MixingOrder == EFactorMixingOrder::EnBlock ? 1 : 0);
		NrEnBlockFactors += (Factor->MixingOrder == EFactorMixingOrder::InOrder ? 1 : 0);
		NrRandomFactors += (Factor->MixingOrder == EFactorMixingOrder::RandomOrder ? 1 : 0);

		if (NrEnBlockFactors > 1)
		{
			//what to actually do when more than one factor wants that???
			FSFUtils::OpenMessageBox(
				"[USFStudyPhase::PhaseValid] " + Factor->FactorName + " in phase " + PhaseName +
				" is already the second enBlock factor, how should that work? If you know, implement ;-)", true);
			return false;
		}

		NrNonCombinedFactors += (Factor->bNonCombined ? 1 : 0);
	}

	if (NrNonCombinedFactors == Factors.Num())
	{
		FSFUtils::OpenMessageBox(
			"[USFStudyPhase::PhaseValid] Phase " + PhaseName + " needs to have at least 1 not-nonCombined factor!", true);
		return false;
	}

	return true;
}

TArray<USFCondition*> USFStudyPhase::GenerateConditions(int ParticipantNr, int PhaseIndex)
{
	//we need the phase index, because we additionally use this to seed the Latin Square randomization so tow identical phases would have different orders

	// first restructure factors, such that:
	// - a potential enBlock factor is the first one
	TArray<USFStudyFactor*> SortedFactors = SortFactors();
	const bool bHasEnBlock = SortedFactors[0]->MixingOrder == EFactorMixingOrder::EnBlock
		|| SortedFactors[0]->MixingOrder == EFactorMixingOrder::InOrder;

	TArray<USFCondition*> Conditions;
	int NumberOfConditions = 1;
	for (const USFStudyFactor* Factor : SortedFactors)
	{
		if (Factor->bNonCombined)
		{
			continue; //those are not part of the condition
		}
		if (Factor->Type == EFactorType::Between)
		{
			continue; //each participant will only see one level of this factor
		}
		NumberOfConditions *= Factor->Levels.Num();
	}


	// ****************************
	// Generate Condition Indices
	// ****************************

	//create an array holding for each condition an array of each factors' level index
	TArray<TArray<int>> ConditionsIndices;
	ConditionsIndices.Reserve(NumberOfConditions * NumberOfRepetitions);
	CreateAllConditionsRecursively(0, {}, SortedFactors, ParticipantNr, ConditionsIndices);

	if (TypeOfRepetition == EPhaseRepetitionType::FullyRandom)
	{
		//simply copy the conditions before shuffling, but always the same next to each other
		//otherwise the latin square had problems due to its modulo nature (still due to doubling conditions not perfect)
		TArray<TArray<int>> ConditionsIndicesCopy = ConditionsIndices;
		ConditionsIndices.Empty();
		for (int i = 0; i < ConditionsIndicesCopy.Num(); ++i)
		{
			for (int r = 0; r < NumberOfRepetitions; ++r)
			{
				TArray<int> ConditionIndices = ConditionsIndicesCopy[i]; //make a copy
				ConditionsIndices.Add(ConditionIndices);
			}
		}
	}


	// ****************************
	//          Randomize
	// ****************************

	int NrLatinSqConditions = ConditionsIndices.Num();
	int enBlockConditions = 1;
	if (bHasEnBlock)
	{
		//that first factor (enBlock) is not shuffled since it is en block already by construction
		//never the less, we still have to change the order within the block, that's what we use LatinSquareRndReOrderEnBlock for
		enBlockConditions = SortedFactors[0]->Levels.Num();
		NrLatinSqConditions /= enBlockConditions;
	}

	TArray<TArray<int>> ConditionsIndicesCopy = ConditionsIndices;
	ConditionsIndices.Empty();

	// for fully random repetitions were already added above and for same order will be farther below,
	// so only set NrDifferentOrderRepetitions if we are repeating DifferentOrder
	const int NrDifferentOrderRepetitions =
		TypeOfRepetition == EPhaseRepetitionType::DifferentOrder ? NumberOfRepetitions : 1;
	for (int Repetition = 0; Repetition < NrDifferentOrderRepetitions; ++Repetition)
	{
		const TArray<int> LatinSquareRndReOrderEnBlock = USFStudyFactor::GenerateLatinSquareOrder(
			ParticipantNr + PhaseIndex + Repetition, enBlockConditions);
		for (int i = 0; i < enBlockConditions; ++i)
		{
			//have a different reshuffling for every enBlock block
			const TArray<int> LatinSquareRndReOrder = USFStudyFactor::GenerateLatinSquareOrder(
				ParticipantNr + PhaseIndex + Repetition + i, NrLatinSqConditions);

			// if we have enBlockConditions>1 we need to copy and shuffle the whole enBlock Block, otherwise the i loop is trivially run once only
			for (int j = 0; j < LatinSquareRndReOrder.Num(); ++j)
			{
				if (SortedFactors[0]->MixingOrder == EFactorMixingOrder::EnBlock)
				{
					ConditionsIndices.Add(ConditionsIndicesCopy[
						enBlockConditions * LatinSquareRndReOrder[j] + LatinSquareRndReOrderEnBlock[i]
					]);
				}
				else
				{
					ConditionsIndices.Add(ConditionsIndicesCopy[
						enBlockConditions * LatinSquareRndReOrder[j] + i
					]);
				}
			}
		}
	}

	// ****************************
	//   Add SameOrder repetitions
	// ****************************

	if (TypeOfRepetition == EPhaseRepetitionType::SameOrder)
	{
		const int NrConditions = ConditionsIndices.Num();
		for (int r = 0; r < NumberOfRepetitions - 1; ++r)
		{
			for (int i = 0; i < NrConditions; ++i)
			{
				TArray<int> ConditionIndices = ConditionsIndices[i]; //make a copy
				ConditionsIndices.Add(ConditionIndices);
			}
		}
	}


	// ****************************
	//   Add non-combined factors
	// ****************************

	for (int i = 0; i < SortedFactors.Num(); ++i)
	{
		const USFStudyFactor* Factor = SortedFactors[i];
		if (!Factor->bNonCombined)
		{
			continue;
		}

		TArray<int> LatinSquare = USFStudyFactor::GenerateLatinSquareOrder(ParticipantNr + PhaseIndex, Factor->Levels.Num());
		if (LatinSquare.Num() < ConditionsIndices.Num())
		{
			FSFLoggingUtils::Log(
				"[USFStudyPhase::GenerateConditions] nonCombined factor levels will be repeated, since factor " + Factor->
				FactorName + " only has " + FString::FromInt(Factor->Levels.Num()) + " levels but there are " +
				FString::FromInt(ConditionsIndices.Num()) + " conditions.");
		}
		for (int j = 0; j < ConditionsIndices.Num(); ++j)
		{
			check(ConditionsIndices[j][i]==-1);

			//repeat the latin square if it does not provide enough levels
			ConditionsIndices[j][i] = LatinSquare[j % Factor->Levels.Num()];
		}
	}


	// ****************************
	//   Create actual conditions
	// ****************************

	Conditions.Empty();
	Conditions.Reserve(ConditionsIndices.Num());

	for (TArray<int> ConditionIndices : ConditionsIndices)
	{
		USFCondition* Condition = NewObject<USFCondition>();
		Condition->Generate(PhaseName, ConditionIndices, SortedFactors, DependentVariables);
		Conditions.Add(Condition);
	}

	return Conditions;
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
		FSFLoggingUtils::Log("[USFStudyPhase::GetAsJson] unknown TypeOfRepetition!", true);
	}

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
		FSFLoggingUtils::Log("[USFStudyPhase::FromJson] unknown TypeOfRepetition: " + RepetitionTypeStr, true);
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

#if WITH_EDITOR
bool USFStudyPhase::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty->GetFName() == "TypeOfRepetition" && NumberOfRepetitions <= 1)
	{
		return false;
	}
	return true;
}
#endif

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

TArray<USFStudyFactor*> USFStudyPhase::SortFactors() const
{
	//puts the enBlock factor first!
	TArray<USFStudyFactor*> SortedFactors;

	for (USFStudyFactor* Factor : Factors)
	{
		if (Factor->MixingOrder == EFactorMixingOrder::RandomOrder)
		{
			SortedFactors.Add(Factor);
		}
		else if (Factor->MixingOrder == EFactorMixingOrder::EnBlock || Factor->MixingOrder == EFactorMixingOrder::InOrder)
		{
			//putting it as first factor in
			SortedFactors.Insert(Factor, 0);
		}
		else
		{
			FSFLoggingUtils::Log("[USFStudyPhase::SortFactors] Unknown MixingOrder!", true);
		}
	}
	return SortedFactors;
}

void USFStudyPhase::CreateAllConditionsRecursively(int Index, TArray<int> TmpOrderPart,
                                                   TArray<USFStudyFactor*>& InSortedFactors, int ParticipantID,
                                                   TArray<TArray<int>>& OutOrdersIndices) const
{
	if (Index >= InSortedFactors.Num())
	{
		OutOrdersIndices.Add(TmpOrderPart);
		return;
	}

	if (InSortedFactors[Index]->bNonCombined)
	{
		//simply go on with next factor and add a placeholder "-1"
		TmpOrderPart.Add(-1);
		CreateAllConditionsRecursively(Index + 1, TmpOrderPart, InSortedFactors, ParticipantID, OutOrdersIndices);
		return;
	}

	if (InSortedFactors[Index]->Type == EFactorType::Between)
	{
		//participants only see one level of this factor, set it, and go on
		TmpOrderPart.Add(ParticipantID % InSortedFactors[Index]->Levels.Num());
		CreateAllConditionsRecursively(Index + 1, TmpOrderPart, InSortedFactors, ParticipantID, OutOrdersIndices);
		return;
	}

	for (int i = 0; i < InSortedFactors[Index]->Levels.Num(); ++i)
	{
		TArray<int> Order = TmpOrderPart;
		Order.Add(i);
		CreateAllConditionsRecursively(Index + 1, Order, InSortedFactors, ParticipantID, OutOrdersIndices);
	}
}
