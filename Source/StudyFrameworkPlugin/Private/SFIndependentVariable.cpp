#include "SFIndependentVariable.h"

#include "Dom/JsonObject.h"
#include "Help/SFUtils.h"


USFIndependentVariable::USFIndependentVariable()
{
}

TSharedPtr<FJsonObject> USFIndependentVariable::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField("Name", Name);
	Json->SetStringField("ValType", GetValTypeAsString());
	Json->SetBoolField("AskedAtBeginning", bAskedAtBeginning);
	Json->SetStringField("Prompt", Prompt);

	TArray<TSharedPtr<FJsonValue>> OptionsJson;
	for (const FString& Option : Options)
	{
		TSharedRef<FJsonValueString> JsonValue = MakeShared<FJsonValueString>(Option);
		OptionsJson.Add(JsonValue);
	}
	Json->SetArrayField("Options", OptionsJson);

	return Json;
}

void USFIndependentVariable::FromJson(TSharedPtr<FJsonObject> Json)
{
	Name = Json->GetStringField("Name");
	SetValTypeFromString(Json->GetStringField("ValType"));
	bAskedAtBeginning = Json->GetBoolField("AskedAtBeginning");
	Prompt = Json->GetStringField("Prompt");

	Options.Empty();
	TArray<TSharedPtr<FJsonValue>> OptionsJson = Json->GetArrayField("Options");
	for (auto Option : OptionsJson) {
		Options.Add(Option->AsString());
	}
}

FString USFIndependentVariable::GetValTypeAsString() const
{
	switch (ValueType) {
	default:
	case EValType::TEXT:
		return "Text";
	case EValType::MULTIPLECHOICE:
		return "Multiple Choice";
	}
}

void USFIndependentVariable::SetValTypeFromString(const FString& Str)
{
	if (Str == "Text") {
		ValueType = EValType::TEXT;
	}
	else if (Str == "Multiple Choice") {
		ValueType = EValType::MULTIPLECHOICE;
	}
}

#if WITH_EDITOR
bool USFIndependentVariable::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty->GetFName() == "Options" && ValueType == EValType::TEXT)
	{
		return false;
	}
	return true;
}
#endif
