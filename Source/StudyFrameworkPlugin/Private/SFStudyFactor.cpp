#include "SFStudyFactor.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

#include "Help/SFUtils.h"


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

void USFStudyFactor::FromJson(TSharedPtr<FJsonObject> Json)
{

	FactorName = Json->GetStringField("FactorName");

	TArray<TSharedPtr<FJsonValue>> LevelsArray = Json->GetArrayField("Levels");
	Levels.Empty();
	for(auto Level : LevelsArray){
		Levels.Add(Level->AsString());
	}

	FString MixingOrderStr = Json->GetStringField("MixingOrder");
	if(MixingOrderStr == "EnBlock")
	{
		MixingOrder = EFactorMixingOrder::EnBlock;
	}
	else if(MixingOrderStr == "RandomOrder")
	{
		MixingOrder = EFactorMixingOrder::RandomOrder;
	}
	else
	{
		FSFUtils::Log("[USFStudyFactor::FromJson] unknown MixingOrder: "+MixingOrderStr, true);
	}

	FString TypeStr = Json->GetStringField("Type");
	if(TypeStr == "Within")
	{
		Type = EFactorType::Within;
	}
	else if(TypeStr == "Between")
	{
		Type = EFactorType::Between;
	}
	else
	{
		FSFUtils::Log("[USFStudyFactor::FromJson] unknown Type: "+TypeStr, true);
	}
}

