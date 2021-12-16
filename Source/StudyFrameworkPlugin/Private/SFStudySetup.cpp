#include "SFStudySetup.h"

#include "SFGameInstance.h"
#include "Help/SFUtils.h"


ASFStudySetup::ASFStudySetup()
{
}

void ASFStudySetup::BeginPlay()
{
}

void ASFStudySetup::PostInitProperties()
{
	LoadFromJson();
	Super::PostInitProperties();
}



void ASFStudySetup::PostEditChangeProperty(FPropertyChangedEvent& MovieSceneBlends)
{
	SaveToJson();
	if(!ContainsNullptrInArrays()){
		LoadFromJson();
		// otherwise you get "Graph is linked to external private object" error
		// but we only do that if !ContainsNullptrInArrays, otherwise you cannot add
		// new elements to the array, since the nullptr are not written to the json
	}
	Super::PostEditChangeProperty(MovieSceneBlends);
}

USFStudyPhase* ASFStudySetup::AddStudyPhase(FString InPhaseName)
{
	USFStudyPhase* Phase = NewObject<USFStudyPhase>(this);
	Phase->PhaseName = InPhaseName;
	Phases.Add(Phase);
	return Phase;
}

void ASFStudySetup::AddActorForEveryLevelInEveryPhase(TSubclassOf<AActor> Actor)
{
	SpawnInEveryPhase.Add(Actor);
}

void ASFStudySetup::AddActorForEveryLevelInEveryPhaseBlueprint(const FString& BlueprintPath, const FString& BlueprintName)
{
	SpawnInEveryPhase.Add(FSFUtils::GetBlueprintClass(BlueprintName, BlueprintPath));
}

bool ASFStudySetup::CheckPhases()
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

TArray<USFCondition*> ASFStudySetup::GetAllConditionsForRun(int RunningParticipantNumber)
{

	if (!CheckPhases())
	{
		FSFUtils::Log("[ASFStudySetup::GetAllConditionsForRun]: Not all Phases are valid", true);
		return TArray<USFCondition*>();
	}

	//TODO: use the RunningParticipantNumber, e.g. for latin square and between factors
	TArray<USFCondition*> Conditions;
	for(USFStudyPhase* Phase : Phases)
	{
		Conditions.Append(Phase->GenerateConditions());
	}
	for(USFCondition* Condition : Conditions)
	{
		Condition->SpawnInThisCondition.Append(SpawnInEveryPhase);
	}
	return Conditions;
}

int ASFStudySetup::GetNumberOfPhases()
{
	return Phases.Num();
}

USFStudyPhase* ASFStudySetup::GetPhase(int Index)
{
	return Phases[Index];
}

TArray<TSubclassOf<AActor>> ASFStudySetup::GetSpawnActors() const
{
	return SpawnInEveryPhase;
}

TSharedPtr<FJsonObject> ASFStudySetup::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	//Phases
	TArray<TSharedPtr<FJsonValue>> PhasesArray;
	for (USFStudyPhase* Phase : Phases)
	{
		if(!Phase)
			continue;
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Phase->GetAsJson());
		PhasesArray.Add(JsonValue);
	}
	Json->SetArrayField("Phases", PhasesArray);

	// SpawnInEveryMapOfThisPhase
	TArray<TSharedPtr<FJsonValue>> SpawnActorsArray;
	for(TSubclassOf<AActor> Class : SpawnInEveryPhase){
		if(!Class)
			continue;
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetStringField("ClassName", Class.Get()->GetName());
		JsonObject->SetStringField("ClassPath", Class.Get()->GetPathName());
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(JsonObject);
		SpawnActorsArray.Add(JsonValue);
	}
	Json->SetArrayField("SpawnInEveryPhase",SpawnActorsArray);

	Json->SetObjectField("FadeConfig", FadeConfig.GetAsJson());
	
	return Json;
}

void ASFStudySetup::FromJson(TSharedPtr<FJsonObject> Json)
{
	Phases.Empty();
	TArray<TSharedPtr<FJsonValue>> PhasesArray = Json->GetArrayField("Phases");
	for (auto PhaseJson : PhasesArray)
	{
		USFStudyPhase* Phase = NewObject<USFStudyPhase>(this);
		Phase->FromJson(PhaseJson->AsObject());
		Phases.Add(Phase);
	}

	//TODO SpawnActor

	FadeConfig.FromJson(Json->GetObjectField("FadeConfig"));
	
}

void ASFStudySetup::LoadFromJson()
{
	TSharedPtr<FJsonObject> Json = FSFUtils::ReadJsonFromFile(JsonFile);
	if(Json){
		FromJson(Json);
	}
}

void ASFStudySetup::SaveToJson() const
{
	TSharedPtr<FJsonObject> Json = GetAsJson();
	FSFUtils::WriteJsonToFile(Json, JsonFile);
}

bool ASFStudySetup::ContainsNullptrInArrays()
{
	for(USFStudyPhase* Phase : Phases)
	{
		if(Phase==nullptr || Phase->ContainsNullptrInArrays())
		{
			return true;
		}
	}
	return false;
}
