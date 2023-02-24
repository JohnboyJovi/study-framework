#include "SFDependentVariable.h"

#include "Dom/JsonObject.h"
#include "Help/SFUtils.h"
#include "Logging/SFLoggingUtils.h"


USFDependentVariable::USFDependentVariable()
{
}

TSharedPtr<FJsonObject> USFDependentVariable::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField("Name", Name);
	Json->SetBoolField("Required", bRequired);
	Json->SetStringField("Type", "USFDependentVariable");

	return Json;
}

USFDependentVariable* USFDependentVariable::FromJson(TSharedPtr<FJsonObject> Json, UObject* Outer)
{
	USFDependentVariable* DependentVariable = nullptr;
	FString Type = Json->GetStringField("Type");
	if (Type == "USFDependentVariable" || Type.IsEmpty())
	{
		DependentVariable = NewObject<USFDependentVariable>(Outer);
	}
	else if (Type == "USFMultipleTrialDependentVariable")
	{
		DependentVariable = NewObject<USFMultipleTrialDependentVariable>(Outer);
	}
	if (!DependentVariable)
	{
		FSFLoggingUtils::Log("[USFDependentVariable::FromJson] Unknown Dependent Variable Type: " + Type, true);
		return nullptr;
	}
	DependentVariable->FromJsonInternal(Json);
	return DependentVariable;
}

void USFDependentVariable::FromJsonInternal(TSharedPtr<FJsonObject> Json)
{
	Name = Json->GetStringField("Name");
	bRequired = Json->GetBoolField("Required");
}

