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

bool USFStudyPhase::PhaseValid() const
{
	USFMapFactor* MapFactor = GetMapFactor();
	if (MapFactor == nullptr || MapFactor->Levels.Num() == 0)
	{
		FSFUtils::OpenMessageBox("Phase " + PhaseName + " is invalid, since no map is set!", true);
		return false;
	}

	int NrEnBlockFactors = 0;
	int NrInOrderFactors = 0;
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
		NrInOrderFactors += (Factor->MixingOrder == EFactorMixingOrder::InOrder ? 1 : 0);
		NrRandomFactors += (Factor->MixingOrder == EFactorMixingOrder::RandomOrder ? 1 : 0);

		if (NrEnBlockFactors > 1)
		{
			//what to actually do when more than one factor wants that???
			FSFUtils::OpenMessageBox(
				"[USFStudyPhase::PhaseValid] " + Factor->FactorName + " in phase " + PhaseName +
				" is already the second enBlock factor, \"nested\" enBlock factors are not supported since they don't seem needed", true);
			return false;
		}

		NrNonCombinedFactors += (Factor->bNonCombined ? 1 : 0);
	}

	if (NrEnBlockFactors + NrInOrderFactors > 1)
	{
		FSFLoggingUtils::Log("[USFStudyPhase::PhaseValid] there is a \"nested\" inOrder factor, globally its levels will not be in order but per level of the prior inOrder/enBlock factor.");
	}

	if (NrNonCombinedFactors == Factors.Num())
	{
		FSFUtils::OpenMessageBox(
			"[USFStudyPhase::PhaseValid] Phase " + PhaseName + " needs to have at least 1 not-nonCombined factor!", true);
		return false;
	}

	return true;
}

TArray<USFCondition*> USFStudyPhase::GenerateConditions(int ParticipantSequenceNr, int PhaseIndex)
{
	//we need the phase index, because we additionally use this to seed the Latin Square randomization so two identical phases would have different orders

	// first restructure factors, such that:
	// - a potential enBlock factor is the first one
	// - then follow InOrder factors
	// - then Random factors
	// - and in the end non-combined factors
	TArray<USFStudyFactor*> SortedFactors = SortFactors();
	const bool bHasEnBlock = SortedFactors[0]->MixingOrder == EFactorMixingOrder::EnBlock;

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

	//create an array holding for each condition an array of each factors' level index (leaving -1 for nonCombined factors, they will be considered later)
	TArray<TArray<int>> ConditionsIndices;
	ConditionsIndices.Reserve(NumberOfConditions);
	CreateAllConditionsRecursively(0, {}, SortedFactors, ParticipantSequenceNr, ConditionsIndices);


	// ****************************
	//          Randomize
	// ****************************

	TArray<TArray<int>> ConditionsIndicesCopy = ConditionsIndices;
	ConditionsIndices.Empty();

	//compute what factors we have to consider:
	int NumRandomConditions = 1;
	int NumEnBlockLevels = 1;
	int NumInOrderLevels = 1;

	for (USFStudyFactor* Factor : SortedFactors)
	{
		if (Factor->bNonCombined) {
			continue;
		}
		else if (Factor->MixingOrder == EFactorMixingOrder::EnBlock) {
			NumEnBlockLevels *= Factor->Levels.Num();
		}
		else if (Factor->MixingOrder == EFactorMixingOrder::InOrder) {
			NumInOrderLevels *= Factor->Levels.Num();
		}
		else if (Factor->MixingOrder == EFactorMixingOrder::RandomOrder) {
			NumRandomConditions *= Factor->Levels.Num();
		}
	}


	// create shuffling of enBlock factor, trivial case ({0}) if we do not have an enBlock factor
	// we devide the participantNr by NumRandomConditions so we do not progress at the same speed, otherwise for simple setup (2-level enBlock, 2-level Random):
	// a1 a2 b2 b1
	// b2 b1 a1 a2 (and then repeating)
	// in case NumRandomConditions is uneven even devide by 2*NumRandomConditions since we have twice as many rows in the Latin Square
	const TArray<int> EnBlockLatinSquare = USFStudyFactor::GenerateLatinSquareOrder(
		ParticipantSequenceNr / (NumRandomConditions*(1 + NumRandomConditions%2)) + PhaseIndex, NumEnBlockLevels);

	for (int EnBlockLevel = 0; EnBlockLevel < NumEnBlockLevels; EnBlockLevel++)
	{
		for (int InOrderLevel = 0; InOrderLevel < NumInOrderLevels; InOrderLevel++)
		{
			const TArray<int> LatinSquare = USFStudyFactor::GenerateLatinSquareOrder(
				ParticipantSequenceNr + PhaseIndex + EnBlockLevel + InOrderLevel, NumRandomConditions);
			//we use all EnBlockLevel + InOrderLevel also for "seeding" so it is not repetitive
			for (int RandomLevel = 0; RandomLevel < LatinSquare.Num(); RandomLevel++)
			{
				ConditionsIndices.Add(ConditionsIndicesCopy[
					NumInOrderLevels * NumRandomConditions * EnBlockLatinSquare[EnBlockLevel]
						+ NumRandomConditions * InOrderLevel
						+ LatinSquare[RandomLevel]
				]);
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

		TArray<int> RandomOrder = {};
		int RandomOrderIndex = 0;
		if (Factor->Levels.Num() < ConditionsIndices.Num())
		{
			FSFLoggingUtils::Log(
				"[USFStudyPhase::GenerateConditions] nonCombined factor levels will be repeated, since factor " + Factor->
				FactorName + " only has " + FString::FromInt(Factor->Levels.Num()) + " levels but there are " +
				FString::FromInt(ConditionsIndices.Num()) + " conditions.");
		}
		for (int j = 0; j < ConditionsIndices.Num(); ++j)
		{
			check(ConditionsIndices[j][i]==-1);

			if(RandomOrderIndex >= RandomOrder.Num())
			{
				//generate a new random order (since we maybe have to repeat the levels of this factor and then each repetition should be random
				RandomOrder = USFStudyFactor::GenerateRandomOrder(ParticipantSequenceNr + PhaseIndex + i + j, Factor->Levels.Num());
				RandomOrderIndex = 0;
			}

			if(Factor->NonCombinedMixingOrder == ENonCombinedFactorMixingOrder::RandomOrder)
			{
				ConditionsIndices[j][i] = RandomOrder[RandomOrderIndex++];
			}
			else if (Factor->NonCombinedMixingOrder == ENonCombinedFactorMixingOrder::InOrder)
			{
				ConditionsIndices[j][i] = RandomOrderIndex++;
			}
			else
			{
				FSFLoggingUtils::Log(
					"[USFStudyPhase::GenerateConditions] unknown non-combined mixing order!");
			}
			
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
		USFDependentVariable* DependentVariable = USFDependentVariable::FromJson(Var->AsObject(), this);
		DependentVariables.Add(DependentVariable);
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

TArray<USFStudyFactor*> USFStudyPhase::SortFactors() const
{
	//puts the enBlock factor first! (in PhaseValid() it is checked that max one exists!)
	//then it puts th inOrder factors, then random factors and in the end nonCombined factors

	TArray<USFStudyFactor*> EnBlockFactor;
	TArray<USFStudyFactor*> InOrderFactors;
	TArray<USFStudyFactor*> RandomFactors;
	TArray<USFStudyFactor*> NonCombinedFactors;
	
	

	for (USFStudyFactor* Factor : Factors)
	{
		if (Factor->bNonCombined)
		{
			NonCombinedFactors.Add(Factor);
		}
		else if (Factor->MixingOrder == EFactorMixingOrder::RandomOrder)
		{
			RandomFactors.Add(Factor);
		}
		else if (Factor->MixingOrder == EFactorMixingOrder::InOrder)
		{
			InOrderFactors.Add(Factor);
		}
		else if (Factor->MixingOrder == EFactorMixingOrder::EnBlock)
		{
			if(EnBlockFactor.Num()!=0)
			{
				FSFLoggingUtils::Log("[USFStudyPhase::SortFactors] found more than one EnBlock factor!", true);
			}
			EnBlockFactor.Add(Factor);
		}
		else
		{
			FSFLoggingUtils::Log("[USFStudyPhase::SortFactors] Unknown MixingOrder!", true);
		}
	}
	TArray<USFStudyFactor*> SortedFactors = EnBlockFactor;
	SortedFactors.Append(InOrderFactors);
	SortedFactors.Append(RandomFactors);
	SortedFactors.Append(NonCombinedFactors);

	return SortedFactors;
}

void USFStudyPhase::CreateAllConditionsRecursively(int Index, TArray<int> TmpOrderPart,
                                                   TArray<USFStudyFactor*>& InSortedFactors, int ParticipantSequenceNumber,
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
		CreateAllConditionsRecursively(Index + 1, TmpOrderPart, InSortedFactors, ParticipantSequenceNumber, OutOrdersIndices);
		return;
	}

	if (InSortedFactors[Index]->Type == EFactorType::Between)
	{
		//participants only see one level of this factor, set it, and go on
		TmpOrderPart.Add(ParticipantSequenceNumber % InSortedFactors[Index]->Levels.Num());
		CreateAllConditionsRecursively(Index + 1, TmpOrderPart, InSortedFactors, ParticipantSequenceNumber, OutOrdersIndices);
		return;
	}

	for (int i = 0; i < InSortedFactors[Index]->Levels.Num(); ++i)
	{
		TArray<int> Order = TmpOrderPart;
		Order.Add(i);
		CreateAllConditionsRecursively(Index + 1, Order, InSortedFactors, ParticipantSequenceNumber, OutOrdersIndices);
	}
}
