#include "SFStudyFactor.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

#include "SFUtils.h"


USFStudyFactor::USFStudyFactor()
{
}

TSharedPtr<FJsonObject> USFStudyFactor::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField("FactorName", FactorName);

	TArray<TSharedPtr<FJsonValue>> LevelsArray;

	for(FString Level : Levels){
		TSharedPtr<FJsonValueString> JsonValue = MakeShared<FJsonValueString>(Level);
		LevelsArray.Add(JsonValue);
	}
	Json->SetArrayField("Levels",LevelsArray);

	switch (MixingOrder)
	{
	case EFactorMixingOrder::EnBlock:
		Json->SetStringField("MixingOrder", "EnBlock");
		break;
	case EFactorMixingOrder::RandomOrder:
		Json->SetStringField("MixingOrder", "RandomOrder");
		break;
	default:
		FSFUtils::Log("[USFStudyFactor::GetAsJson] unknown MixingOrder!", true);
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
		FSFUtils::Log("[USFStudyFactor::GetAsJson] unknown FactorType!", true);
	}

	return Json;
}

