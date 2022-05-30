#include "SFStudySetup.h"

#include "SFGameInstance.h"
#include "HAL/FileManagerGeneric.h"
#include "Help/SFUtils.h"


ASFStudySetup::ASFStudySetup()
{
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> IconTextureObject(TEXT("/StudyFrameworkPlugin/SetupIcon"));
	// We need a scene component to attach Icon sprite
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	RootComponent = SceneComponent;
	RootComponent->Mobility = EComponentMobility::Static;
	USFLogObject* myLogObject = CreateDefaultSubobject<USFLogObject>(TEXT("SFLogObject"));
	LogObject = myLogObject;
	LogObject->Initialize();
	ActorsToLog = LogObject->LoggingInfo;

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->Sprite = IconTextureObject.Get(); // Get the sprite texture from helper class object
		SpriteComponent->SetupAttachment(RootComponent); // Attach sprite to scene component
		SpriteComponent->Mobility = EComponentMobility::Static;
	}
#endif // WITH_EDITORONLY_DATA
}

void ASFStudySetup::BeginPlay()
{
	Super::BeginPlay();
}

void ASFStudySetup::PostLoad()
{
	Super::PostLoad();
	LoadFromJson();
}

void ASFStudySetup::PreSave(const ITargetPlatform* TargetPlatform)
{
	SaveToJson();
	if (!ContainsNullptrInArrays())
	{
		LoadFromJson();
		// otherwise you get "Graph is linked to external private object" error
		// but we only do that if !ContainsNullptrInArrays, otherwise you cannot add
		// new elements to the array, since the nullptr are not written to the json
	}
	Super::PreSave(TargetPlatform);
}

#if WITH_EDITOR
void ASFStudySetup::PostEditChangeProperty(FPropertyChangedEvent& MovieSceneBlends)
{
	//not needed anymore (done on saving map and on clicking the respective button for full control)
	//SaveToJson();
	Super::PostEditChangeProperty(MovieSceneBlends);
}
#endif

USFStudyPhase* ASFStudySetup::AddStudyPhase(FString InPhaseName)
{
	USFStudyPhase* Phase = NewObject<USFStudyPhase>(this);
	Phase->PhaseName = InPhaseName;
	Phases.Add(Phase);
	return Phase;
}

bool ASFStudySetup::CheckPhases() const
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

void ASFStudySetup::GenerateTestStudyRuns() const
{
	for (int ParticipantID = 0; ParticipantID < NrOfRunsToGenerate; ++ParticipantID)
	{
		const TArray<USFCondition*> Conditions = GetAllConditionsForRun(ParticipantID);
		USFParticipant* TmpParticipant = NewObject<USFParticipant>();
		TmpParticipant->Initialize(ParticipantID);
		TmpParticipant->SetStudyConditions(Conditions); //this also saves it to json
	}
}

void ASFStudySetup::ClearStudyResults() const
{
	const FText MessageText = FText::FromString(
		"You are about to delete all results. This is meant for deleting produced data during debug. So only use BEFORE starting the actual study!\n\nAre you sure you want to proceed?");
	const FText MessageTitle = FText::FromString("CAUTION: Delete gathered data?");
	const EAppReturnType::Type Answer = FMessageDialog::Open(EAppMsgType::YesNo, EAppReturnType::No, MessageText,
	                                                         &MessageTitle);
	if (Answer != EAppReturnType::Yes)
	{
		return;
	}

	auto DeleteFolder = [](FString FolderName)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString CompletePath = FPaths::ProjectDir() + "StudyFramework/" + FolderName;
		if (PlatformFile.DirectoryExists(*CompletePath))
		{
			FFileManagerGeneric::Get().DeleteDirectory(*CompletePath, true, true);
		}
	};

	DeleteFolder("StudyRuns");
	DeleteFolder("Results");
}

TArray<USFCondition*> ASFStudySetup::GetAllConditionsForRun(int RunningParticipantNumber) const
{
	if (!CheckPhases())
	{
		FSFUtils::Log("[ASFStudySetup::GetAllConditionsForRun]: Not all Phases are valid", true);
		return TArray<USFCondition*>();
	}

	TArray<USFCondition*> Conditions;
	for (USFStudyPhase* Phase : Phases)
	{
		Conditions.Append(Phase->GenerateConditions(RunningParticipantNumber));
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


TSharedPtr<FJsonObject> ASFStudySetup::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

	//Phases
	TArray<TSharedPtr<FJsonValue>> PhasesArray;
	for (USFStudyPhase* Phase : Phases)
	{
		if (!Phase)
			continue;
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Phase->GetAsJson());
		PhasesArray.Add(JsonValue);
	}
	Json->SetArrayField("Phases", PhasesArray);

	Json->SetObjectField("FadeConfig", FadeConfig.GetAsJson());
	Json->SetObjectField("ExperimenterViewConfig", ExperimenterViewConfig.GetAsJson());

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
	FadeConfig.FromJson(Json->GetObjectField("FadeConfig"));
	ExperimenterViewConfig.FromJson(Json->GetObjectField("ExperimenterViewConfig"));
}

void ASFStudySetup::LoadFromJson()
{
	TSharedPtr<FJsonObject> Json = FSFUtils::ReadJsonFromFile(JsonFile);
	if (Json)
	{
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
	for (USFStudyPhase* Phase : Phases)
	{
		if (Phase == nullptr || Phase->ContainsNullptrInArrays())
		{
			return true;
		}
	}
	return false;
}
