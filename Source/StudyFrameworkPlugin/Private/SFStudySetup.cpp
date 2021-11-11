#include "SFStudySetup.h"

#include "SFUtils.h"


USFStudySetup::USFStudySetup()
{
}

USFStudyPhase* USFStudySetup::AddStudyPhase(FName PhaseName)
{
	USFStudyPhase* Phase = NewObject<USFStudyPhase>(GetTransientPackage(), PhaseName);
	Phases.Add(Phase);
	return Phase;
}

void USFStudySetup::AddActorForEveryLevelInEveryPhase(TSubclassOf<AActor> Actor)
{
	SpawnInEveryPhase.Add(Actor);
}

void USFStudySetup::AddActorForEveryLevelInEveryPhaseBlueprint(const FString& BlueprintPath, const FString& BlueprintName)
{
	SpawnInEveryPhase.Add(FSFUtils::GetBlueprintClass(BlueprintName, BlueprintPath));
}

bool USFStudySetup::CheckPhases()
{
	for (auto EntryPhase : Phases)
	{
		if (!EntryPhase->PhaseValid())
		{
			return false;
		}
	}

	return true;
}

int USFStudySetup::GetNumberOfPhases()
{
	return Phases.Num();
}

USFStudyPhase* USFStudySetup::GetPhase(int Index)
{
	return Phases[Index];
}

TSharedPtr<FJsonObject> USFStudySetup::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	//Phases
	TArray<TSharedPtr<FJsonValue>> PhasesArray;
	for (USFStudyPhase* Phase : Phases)
	{
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Phase->GetAsJson());
		PhasesArray.Add(JsonValue);
	}
	Json->SetArrayField("Phases", PhasesArray);

	// SpawnInEveryMapOfThisPhase
	TArray<TSharedPtr<FJsonValue>> SpawnActorsArray;
	for(TSubclassOf<AActor> Class : SpawnInEveryPhase){
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetStringField("ClassName", Class.Get()->GetName());
		JsonObject->SetStringField("ClassPath", Class.Get()->GetPathName());
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(JsonObject);
		SpawnActorsArray.Add(JsonValue);
	}
	Json->SetArrayField("SpawnInEveryPhase",SpawnActorsArray);
	
	return Json;
}
