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

void USFMapFactor::FromJson(TSharedPtr<FJsonObject> Json)
{
	Super::FromJson(Json);

	for(FString LevelName : Levels)
	{
		TAssetPtr<UWorld> LevelObj = LoadObject<UWorld>(nullptr, *LevelName, nullptr, LOAD_None);
		Maps.Add(LevelObj.Get());
	}
}

#if WITH_EDITOR
bool USFMapFactor::CanEditChange(const FProperty* InProperty) const
{
	//just use this to not allow editing "Levels" directly at all
	if (InProperty->GetFName() == "Levels")
	{
		return false;
	}
	return Super::CanEditChange(InProperty);
}

void USFMapFactor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.MemberProperty->GetFName() == "Maps")
	{
		//adapt levels accordingly, since that is what is internaly used
		Levels.Empty();
		for(TAssetPtr<UWorld> Map : Maps)
		{
			FSoftObjectPath Path(Map.Get());
			FString PathString = Path.GetAssetPathString();
			int32 DotIndex;
			PathString.FindLastChar('.', DotIndex);
			PathString.LeftChopInline(PathString.Len() - DotIndex);
			Levels.Add(PathString);
		}
	}
}
#endif

