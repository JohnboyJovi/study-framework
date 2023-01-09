#include "SFStudySetup.h"

#include "SFGameInstance.h"
#include "HAL/FileManagerGeneric.h"
#include "Help/SFUtils.h"
#include "Logging/SFLogObject.h"
#include "Logging/SFLoggingUtils.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Kismet/KismetStringLibrary.h"

ASFStudySetup::ASFStudySetup()
{
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> IconTextureObject(TEXT("/StudyFrameworkPlugin/SetupIcon"));
	// We need a scene component to attach Icon sprite
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	RootComponent = SceneComponent;
	RootComponent->Mobility = EComponentMobility::Static;
	JsonFile = "StudySetup.json";

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

void ASFStudySetup::PostActorCreated()
{
	Super::PostActorCreated();

	// PostActorCreated() is called twice when actor is drag-'n'-dropped into level because:
	// First a preview actor with transient values is created when dragging out of list
	// Then the final actor is created once dropped into map.
	// We only want to execute the code for the latter actor -> Workaround:
	if (!HasAllFlags(RF_Transient))
	{
		int uniqueFileExtension = 0;
		int NumOfDigitsExtension;
		while (FPaths::FileExists(FSFUtils::GetStudyFrameworkPath() + JsonFile))
		{
			NumOfDigitsExtension = FString::FromInt(uniqueFileExtension).Len();
			JsonFile.RemoveFromEnd(".json");

			// Filename ends with number to iterate
			if (JsonFile.Right(NumOfDigitsExtension).IsNumeric())
			{
				uniqueFileExtension = UKismetStringLibrary::Conv_StringToInt(JsonFile.Right(NumOfDigitsExtension));
			}

			// Filename ends with number but with fewer digits, e.g. file9.json exists but not file10.json
			else if (NumOfDigitsExtension > 1)
			{
				uniqueFileExtension = UKismetStringLibrary::Conv_StringToInt(JsonFile.Right(NumOfDigitsExtension - 1));
			}

			// There is no number at the end that should be removed before adding larger number
			else
			{
				JsonFile = JsonFile + "1" + ".json";
				continue;
			}

			JsonFile.RemoveFromEnd(FString::FromInt(uniqueFileExtension));
			JsonFile.AppendInt(uniqueFileExtension + 1);
			JsonFile.Append(".json");
			FSFLoggingUtils::Log("Attempting to use " + JsonFile);
		}
		SaveToJson();
	}
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
	//Re-enabled to avoid accidental data loss
	SaveToJson();
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

	for(FString PhaseName : PhasesToOrderRandomize)
	{
		bool bPhaseExists = false;
		for(int i=0; i<Phases.Num(); ++i)
		{
			if (Phases[i]->PhaseName == PhaseName) bPhaseExists = true;
		}
		if(!bPhaseExists)
		{
			FSFUtils::OpenMessageBox("Phase " + PhaseName + " cannot be randomized in order, since it does not exist!", true);
			return false;
		}
	}

	return true;
}

void ASFStudySetup::GenerateTestStudyRuns() const
{
	TArray<FString> RunStrings;
	for (int ParticipantID = 0; ParticipantID < NrOfRunsToGenerate; ++ParticipantID)
	{
		const TArray<USFCondition*> Conditions = GetAllConditionsForRun(ParticipantID);
		USFParticipant* TmpParticipant = NewObject<USFParticipant>();
		TmpParticipant->Initialize(ParticipantID);
		TmpParticipant->SetStudyConditions(Conditions); //this also saves it to json

		FString RunString = FString::FromInt(ParticipantID);
		for(USFCondition* Condition : Conditions)
		{
			RunString += "\t" + Condition->ToString();
		}
		RunStrings.Add(RunString);
	}
	FFileHelper::SaveStringArrayToFile(RunStrings, *(FPaths::ProjectDir() + "StudyFramework/StudyRuns/GeneratedDebugRuns.txt"));
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
	DeleteFolder("StudyLogs");
}

TArray<USFCondition*> ASFStudySetup::ConditionSortingCallback(const TArray<USFCondition*>& Conditions, int ParticipantRunningNumber) const
{
	//in this default implementation we don't do anything here
	return Conditions;
}

TArray<USFCondition*> ASFStudySetup::GetAllConditionsForRun(int RunningParticipantNumber) const
{
	if (!CheckPhases())
	{
		FSFLoggingUtils::Log("[ASFStudySetup::GetAllConditionsForRun]: Not all Phases are valid", true);
		return TArray<USFCondition*>();
	}

	//so we have to potentially swap some phases
	TArray<int> PhasesToShuffleIndices;
	for(int i=0; i<Phases.Num(); ++i)
	{
		if (PhasesToOrderRandomize.Contains(Phases[i]->PhaseName))
		{
			PhasesToShuffleIndices.Add(i);
		}
	}
	TArray<int> LatinSquare = USFStudyFactor::GenerateLatinSquareOrder(RunningParticipantNumber, PhasesToShuffleIndices.Num());

	TArray<USFCondition*> Conditions;
	for (int i=0; i<Phases.Num(); ++i)
	{
		int ActualIndex = i;
		if(PhasesToShuffleIndices.Contains(i))
		{
			//this one needs to be shuffled
			const int IndexInShuffleArray = PhasesToShuffleIndices.Find(i);
			ActualIndex = PhasesToShuffleIndices[LatinSquare[IndexInShuffleArray]];
		}
		Conditions.Append(Phases[ActualIndex]->GenerateConditions(RunningParticipantNumber, ActualIndex));
	}

	Conditions = ConditionSortingCallback(Conditions);

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

	TArray<TSharedPtr<FJsonValue>> PhasesToRandomize;
	for (FString Phase : PhasesToOrderRandomize)
	{
		TSharedRef<FJsonValueString> JsonValue = MakeShared<FJsonValueString>(Phase);
		PhasesToRandomize.Add(JsonValue);
	}
	Json->SetArrayField("PhasesToOrderRandomize", PhasesToRandomize);

	Json->SetObjectField("FadeConfig", FadeConfig.GetAsJson());
	Json->SetObjectField("ExperimenterViewConfig", ExperimenterViewConfig.GetAsJson());
	if(UseGazeTracker == EGazeTrackerMode::NotTracking) Json->SetStringField("UseGazeTracker", "NotTracking");
	if(UseGazeTracker == EGazeTrackerMode::HeadRotationOnly) Json->SetStringField("UseGazeTracker", "HeadRotationOnly");
	if(UseGazeTracker == EGazeTrackerMode::EyeTracking) Json->SetStringField("UseGazeTracker", "EyeTracking");
	

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

	PhasesToOrderRandomize.Empty();
	TArray<TSharedPtr<FJsonValue>> PhasesToRandomize = Json->GetArrayField("PhasesToOrderRandomize");
	for (TSharedPtr<FJsonValue> PhaseJson : PhasesToRandomize)
	{
		PhasesToOrderRandomize.Add(PhaseJson->AsString());
	}

	FadeConfig.FromJson(Json->GetObjectField("FadeConfig"));
	ExperimenterViewConfig.FromJson(Json->GetObjectField("ExperimenterViewConfig"));
	if(Json->GetStringField("UseGazeTracker") == "NotTracking") UseGazeTracker = EGazeTrackerMode::NotTracking;
	if(Json->GetStringField("UseGazeTracker") == "HeadRotationOnly") UseGazeTracker = EGazeTrackerMode::HeadRotationOnly;
	if(Json->GetStringField("UseGazeTracker") == "EyeTracking") UseGazeTracker = EGazeTrackerMode::EyeTracking;
}

void ASFStudySetup::LoadSetupFile()
{
	// OpenFileDialog() requires an array for the return value,
	// but the file picker window only allows one file to be selected,
	// so using SelectedFilePath[0] works fine consistently
	TArray<FString> SelectedFilePath;
	FDesktopPlatformModule::Get()->OpenFileDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr), FString("Select Setup File"), FSFUtils::GetStudyFrameworkPath(),
											FString(""), FString("JSON Files|*.json"), 0, SelectedFilePath);

	if (SelectedFilePath.Num() == 0 || !SelectedFilePath[0].EndsWith(".json"))
	{
		return;
	}
	// Make path relative to ProjectDir/StudyFramework
	if(!FPaths::MakePathRelativeTo(SelectedFilePath[0], *FSFUtils::GetStudyFrameworkPath()))
	{
		FSFLoggingUtils::Log("Was not able to make selected file path relative to working directory. Ensure that the paths share the same root folder (i.e. are located on the same drive)", true);
		return;
	}
	if (JsonFile != SelectedFilePath[0])
	{
		this->Modify(true);
		JsonFile = SelectedFilePath[0];
	}
	LoadFromJson();
}

void ASFStudySetup::SaveSetupFile()
{
	TArray<FString> SelectedFilePath;
	FDesktopPlatformModule::Get()->SaveFileDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr), FString("Save Setup File To"), FSFUtils::GetStudyFrameworkPath(), 
		JsonFile, FString("JSON Files|*.json"), 0, SelectedFilePath);

	if (SelectedFilePath.Num() == 0 || !SelectedFilePath[0].EndsWith(".json"))
	{
		return;
	}

	// Make path relative to ProjectDir/StudyFramework
	if (!FPaths::MakePathRelativeTo(SelectedFilePath[0], *FSFUtils::GetStudyFrameworkPath()))
	{
		FSFLoggingUtils::Log("Was not able to make selected file path relative to working directory. Ensure that the paths share the same root folder (i.e. are located on the same drive)", true);
		return;
	}
	if (JsonFile != SelectedFilePath[0])
	{
		this->Modify(true);
		JsonFile = SelectedFilePath[0];
	}
	SaveToJson();
}

void ASFStudySetup::LoadFromJson()
{
	TSharedPtr<FJsonObject> Json = FSFUtils::ReadJsonFromFile(JsonFile);
	if (Json)
	{
		FromJson(Json);
		FSFLoggingUtils::Log("Loaded setup file " + JsonFile);
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
