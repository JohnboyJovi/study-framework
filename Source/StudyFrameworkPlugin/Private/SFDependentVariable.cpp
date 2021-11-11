#include "SFDependentVariable.h"

#include "SFUtils.h"


USFDependentVariable::USFDependentVariable()
{
}

TSharedPtr<FJsonObject> USFDependentVariable::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField("Name", Name);

	return Json;
}

