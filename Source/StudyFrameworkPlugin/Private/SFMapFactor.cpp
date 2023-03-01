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
		//we had issues with maps being loaded once in the editor and then on starting the study this method
		//was called again in PostLoad() and tried to reload an already loaded map which caused crashes
		//with the if-clause below we do not load the maps when starting the study, where we don't need the
		//asset pointer anyways since the study itself only uses the Levels strings directly
		UWorld* World = GetWorld();
		if(World && World->WorldType != EWorldType::PIE && World->WorldType != EWorldType::Game)
		{
			TAssetPtr<UWorld> LevelObj = StaticLoadObject(UWorld::StaticClass(), nullptr, *LevelName, nullptr, LOAD_Verify | LOAD_Quiet);
			Maps.Add(LevelObj.Get());
			
		}
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

