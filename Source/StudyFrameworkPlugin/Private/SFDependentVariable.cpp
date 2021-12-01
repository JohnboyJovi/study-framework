#include "SFDependentVariable.h"

#include "Dom/JsonObject.h"
#include "Help/SFUtils.h"


USFDependentVariable::USFDependentVariable()
{
}

TSharedPtr<FJsonObject> USFDependentVariable::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField("Name", Name);
	Json->SetBoolField("Required", bRequired);

	return Json;
}

void USFDependentVariable::FromJson(TSharedPtr<FJsonObject> Json)
{
	Name = Json->GetStringField("Name");
	bRequired = Json->GetBoolField("Required");
}

