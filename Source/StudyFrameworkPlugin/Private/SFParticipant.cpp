// Fill out your copyright notice in the Description page of Project Settings.


#include "SFParticipant.h"

#include "SFGameInstance.h"
#include "Help/SFLogger.h"
#include "Help/SFUtils.h"

USFParticipant::USFParticipant()
{
}

USFParticipant::~USFParticipant()
{
}

void USFParticipant::SaveDataArray(FString Where, TArray<FString> Data)
{
	//TODO: what is this used for???
	Logger->SaveDataArray(Where, Data, CurrentPhaseIdx, CurrentPhase->GetCurrentCondition()->ToString());
}


bool USFParticipant::Initialize(FString IdNew, FString JsonFilePath, FString LogName, FString SaveDataLogName)
{
	ParticipantID = IdNew;

	// TODO initialize Logger!!!
	Logger = NewObject<USFLogger>();
	Logger->Initialize(this, JsonFilePath, LogName, SaveDataLogName);

	return true;
}

void USFParticipant::GenerateExecutionJsonFile()
{
	// Create initial Json File
	/*
	 + Phases (object)
	 +     Num (Int)
	 +         5 (Value)
	 +     1 (object)
	 +         Setup (Array)
	 +             2_2_2 (Value)
	 +         Order (FString Array)
	 +             1_1,1_2,2_1,2_2 (Value)

	 + "Data": (object)
	 +     "Phase_1": (object)
	 +         "1_1": (object)
	 -             "NameOfData": (Array)
	 >                  1,2,3,4,5 (Value)
	*/

	MainJsonObject = MakeShared<FJsonObject>();

	/*TSharedPtr<FJsonObject> JsonPhases = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> JsonData = MakeShared<FJsonObject>();

	const int NumPhases = StudySetup->GetNumberOfPhases();

	JsonPhases->SetNumberField("NumberOfPhases", NumPhases);

	TArray<TSharedPtr<FJsonObject>> JsonEachPhase;

	for (int i = 0; i < NumPhases; i++)
	{
		TSharedPtr<FJsonObject> JsonTmpPhases = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> JsonTmpData = MakeShared<FJsonObject>();

		USFStudyPhase* Phase = StudySetup->GetPhase(i);

		// Setup
		TArray<int> SetupInt = Phase->GetFactorsLevelCount();
		TArray<TSharedPtr<FJsonValue>> Setup;
		for (auto Entry : SetupInt)
		{
			TSharedPtr<FJsonValueNumber> Tmp = MakeShared<FJsonValueNumber>(Entry);
			Setup.Add(Tmp);
		}
		JsonTmpPhases->SetArrayField("Setup", Setup);

		// Order
		TArray<FString> OrderFString = FString::Join(Phase->GetOrderStrings();
		TArray<TSharedPtr<FJsonValue>> Order;
		for (auto Entry : OrderFString)
		{
			TSharedPtr<FJsonValueString> Tmp = MakeShared<FJsonValueString>(Entry);
			Order.Add(Tmp);
		}
		JsonTmpPhases->SetArrayField("Order", Setup);

		// Add Phase
		JsonPhases->SetObjectField(FString::FromInt(i), JsonTmpPhases);


		// Data Part
		for (auto Entry : OrderFString)
		{
			TSharedPtr<FJsonObject> Tmp = MakeShared<FJsonObject>();
			JsonTmpData->SetObjectField(Entry, Tmp);
		}

		JsonData->SetObjectField(FString::FromInt(i), JsonTmpData);
	}

	MainJsonObject->SetObjectField("Phases", JsonPhases);
	MainJsonObject->SetObjectField("Data", JsonData);

	FString JsonAsString = FSFUtils::JsonToString(MainJsonObject);

	FSFUtils::Log(JsonAsString, false);*/

	//TODO: recreate!
}

bool USFParticipant::StartStudy(USFStudySetup* InStudySetup)
{
	//TODO: recover from crashed run?
	// If reload an already existing study?
	/*if (FindJsonFile())
	{
		FSFUtils::Log("[USFParticipant::StartStudy()]: Json File found. Loading it now..", false);
		return LoadJsonFile();
	}*/

	StudySetup = InStudySetup;

	if (!StudySetup->CheckPhases())
	{
		FSFUtils::Log("[USFParticipant::StartStudy()]: Not all Phases valid", true);
		return false;
	}


	// Setup order
	for (int PhaseIndex = 0; PhaseIndex < StudySetup->GetNumberOfPhases(); ++PhaseIndex)
	{
		StudySetup->GetPhase(PhaseIndex)->GenerateConditions();
	}

	FSFUtils::Log(
		"[USFParticipant::StartStudy()]: Generated Phases for " + FString::FromInt(StudySetup->GetNumberOfPhases()) +
		" phases",
		false);

	// Create initial Json file
	GenerateExecutionJsonFile();

	// And save it
	Logger->SaveJsonFile(MainJsonObject);

	// Set first phase
	CurrentPhase = StudySetup->GetPhase(0);
	CurrentPhaseIdx = 0;

	return true;
}

FString USFParticipant::NextCondition()
// TODO can maybe be made a schöner function with different if phase finished logic
{
	// Get next Setup
	UpcomingCondition = CurrentPhase->NextCondition();

	if (UpcomingCondition == nullptr)
	{
		if (CurrentPhaseIdx >= (StudySetup->GetNumberOfPhases() - 1)) // So there is no next phase
		{
			FSFUtils::Log("[USFParticipant::NextCondition()]: All setups already ran, EndStudy()", false);

			USFGameInstance::Get()->EndStudy();

			return "";
		}
		else
		{
			CurrentPhase = StudySetup->GetPhase(++CurrentPhaseIdx);

			UpcomingCondition = CurrentPhase->NextCondition();
		}
	}

	const FString LevelName = UpcomingCondition->Map;

	return LevelName;
}

void USFParticipant::EndStudy()
{
}

void USFParticipant::LogData(FString Data)
{
	Logger->LogData(Data);
}

void USFParticipant::CommitData()
{
	Logger->CommitData();
}

TSharedPtr<FJsonObject> USFParticipant::GetJsonFile()
{
	return MainJsonObject;
}

USFStudyPhase* USFParticipant::GetCurrentPhase()
{
	return CurrentPhase;
}

int USFParticipant::GetCurrentPhaseIdx()
{
	return CurrentPhaseIdx;
}

FString USFParticipant::GetID()
{
	return ParticipantID;
}
