#include "SFStudyFactor.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

#include "Logging/SFLoggingUtils.h"


USFStudyFactor::USFStudyFactor()
{
}

TSharedPtr<FJsonObject> USFStudyFactor::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField("FactorName", FactorName);

	TArray<TSharedPtr<FJsonValue>> LevelsArray;

	for (FString Level : Levels)
	{
		TSharedPtr<FJsonValueString> JsonValue = MakeShared<FJsonValueString>(Level);
		LevelsArray.Add(JsonValue);
	}
	Json->SetArrayField("Levels", LevelsArray);

	switch (MixingOrder)
	{
	case EFactorMixingOrder::EnBlock:
		Json->SetStringField("MixingOrder", "EnBlock");
		break;
	case EFactorMixingOrder::RandomOrder:
		Json->SetStringField("MixingOrder", "RandomOrder");
		break;
	case EFactorMixingOrder::InOrder:
		Json->SetStringField("MixingOrder", "InOrder");
		break;
	default:
		FSFLoggingUtils::Log("[USFStudyFactor::GetAsJson] unknown MixingOrder!", true);
	}

	switch (NonCombinedMixingOrder)
	{
	case ENonCombinedFactorMixingOrder::RandomOrder:
		Json->SetStringField("NonCombinedMixingOrder", "RandomOrder");
		break;
	case ENonCombinedFactorMixingOrder::InOrder:
		Json->SetStringField("NonCombinedMixingOrder", "InOrder");
		break;
	default:
		FSFLoggingUtils::Log("[USFStudyFactor::GetAsJson] unknown NonCombinedMixingOrder!", true);
	}

	switch (Type)
	{
	case EFactorType::Within:
		Json->SetStringField("Type", "Within");
		break;
	case EFactorType::Between:
		Json->SetStringField("Type", "Between");
		break;
	default:
		FSFLoggingUtils::Log("[USFStudyFactor::GetAsJson] unknown FactorType!", true);
	}

	Json->SetBoolField("NonCombined", bNonCombined);

	return Json;
}

void USFStudyFactor::FromJson(TSharedPtr<FJsonObject> Json)
{
	FactorName = Json->GetStringField("FactorName");

	TArray<TSharedPtr<FJsonValue>> LevelsArray = Json->GetArrayField("Levels");
	Levels.Empty();
	for (auto Level : LevelsArray)
	{
		Levels.Add(Level->AsString());
	}

	FString MixingOrderStr = Json->GetStringField("MixingOrder");
	if (MixingOrderStr == "EnBlock")
	{
		MixingOrder = EFactorMixingOrder::EnBlock;
	}
	else if (MixingOrderStr == "RandomOrder")
	{
		MixingOrder = EFactorMixingOrder::RandomOrder;
	}
	else if (MixingOrderStr == "InOrder")
	{
		MixingOrder = EFactorMixingOrder::InOrder;
	}
	else
	{
		FSFLoggingUtils::Log("[USFStudyFactor::FromJson] unknown MixingOrder: " + MixingOrderStr, true);
	}

	FString NonCombinedMixingOrderStr = Json->GetStringField("NonCombinedMixingOrder");
	if (NonCombinedMixingOrderStr == "RandomOrder")
	{
		NonCombinedMixingOrder = ENonCombinedFactorMixingOrder::RandomOrder;
	}
	else if (NonCombinedMixingOrderStr == "InOrder")
	{
		NonCombinedMixingOrder = ENonCombinedFactorMixingOrder::InOrder;
	}
	else
	{
		FSFLoggingUtils::Log("[USFStudyFactor::FromJson] unknown NonCombinedMixingOrder: " + MixingOrderStr, true);
	}

	FString TypeStr = Json->GetStringField("Type");
	if (TypeStr == "Within")
	{
		Type = EFactorType::Within;
	}
	else if (TypeStr == "Between")
	{
		Type = EFactorType::Between;
	}
	else
	{
		FSFLoggingUtils::Log("[USFStudyFactor::FromJson] unknown Type: " + TypeStr, true);
	}

	bNonCombined = Json->GetBoolField("NonCombined");
}

#if WITH_EDITOR
bool USFStudyFactor::CanEditChange(const FProperty * InProperty) const
{
	if(InProperty->GetFName()=="MixingOrder" && (bNonCombined || Type==EFactorType::Between))
	{
		return false;
	}
	return true;
}

void USFStudyFactor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	if (Type == EFactorType::Between)
	{
		//especially if still EnBlock is set as MixingOrder that breaks the randomization algorithm
		MixingOrder = EFactorMixingOrder::RandomOrder;
	}
}
#endif

TArray<int> USFStudyFactor::GenerateLatinSquareOrder(int OrderNr, int NrConditions)
{
	// Balanced Latin Square Generator
	// Based on "Bradley, J. V. Complete counterbalancing of immediate sequential effects in a Latin square design. J. Amer. Statist. Ass.,.1958, 53, 525-528. "
	// Adapted from code on https://cs.uwaterloo.ca/~dmasson/tools/latin_square/
	// IMPORTANT: for uneven NrConditions, there 2*NrConditions different orders!
	TArray<int> Result;
	for (int i = 0, j = 0, h = 0; i < NrConditions; ++i)
	{
		int val = 0;
		if (i < 2 || i % 2 != 0)
		{
			val = j++;
		}
		else
		{
			val = NrConditions - h - 1;
			++h;
		}

		int idx = (val + OrderNr) % NrConditions;
		Result.Add(idx);
	}

	if (NrConditions % 2 != 0 && OrderNr % 2 != 0)
	{
		//reverse Result
		TArray<int> TmpResult = Result;
		for (int i = 0; i < TmpResult.Num(); ++i)
		{
			Result[i] = TmpResult[TmpResult.Num() - i - 1];
		}
	}

	return Result;
}

TArray<int> USFStudyFactor::GenerateRandomOrder(int OrderNr, int NrConditions)
{
	//not the most efficient implementation, but straight forward. Efficiency is not so important when creating study runs.
	FRandomStream RNG(OrderNr);
	//Create Ordered List;
	TArray<int> Conditions;
	for (int i = 0; i < NrConditions; ++i)
	{
		Conditions.Add(i);
	}

	TArray<int> RandomConditions;
	for (int i = 0; i < NrConditions; ++i)
	{
		int RandomIndex = RNG.RandRange(0, Conditions.Num() - 1);
		RandomConditions.Add(Conditions[RandomIndex]);
		Conditions.RemoveAt(RandomIndex);
	}
	check(Conditions.Num()==0)
	return RandomConditions;
}
