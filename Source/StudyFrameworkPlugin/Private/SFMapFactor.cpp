#include "SFMapFactor.h"

USFMapFactor::USFMapFactor()
{
}

TSharedPtr<FJsonObject> USFMapFactor::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = Super::GetAsJson();

	Json->SetBoolField("MapFactor", true);

	return Json;
}

