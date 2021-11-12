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
	Logger->SaveDataArray(Where, Data, /*CurrentPhaseIdx*/ 0, Conditions[CurrentConditionIdx]->ToString());
}


bool USFParticipant::Initialize(FString IdNew, FString JsonFilePath, FString LogName, FString SaveDataLogName)
{
	ParticipantID = IdNew;

	// TODO initialize Logger!!!
	Logger = NewObject<USFLogger>();
	Logger->Initialize(this, JsonFilePath, LogName, SaveDataLogName);

	return true;
}

void USFParticipant::GenerateExecutionJsonFile() const
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


	// Conditions order
	Conditions = StudySetup->GetAllConditionsForRun(0); //TODO: we need a running number!

	FSFUtils::Log(
		"[USFParticipant::StartStudy()]: Generated " + FString::FromInt(Conditions.Num()) +
		" conditions for participant " + ParticipantID,
		false);

	// Create initial Json file
	GenerateExecutionJsonFile();

	// Set first condition
	CurrentConditionIdx = -1;

	return true;
}

FString USFParticipant::NextCondition()
{
	// Get next Condition
	if (++CurrentConditionIdx >= Conditions.Num())
	{
		FSFUtils::Log("[USFParticipant::NextCondition()]: All conditions already ran, EndStudy()", false);
		USFGameInstance::Get()->EndStudy();
		return "";
	}
	USFCondition* UpcomingCondition = Conditions[CurrentConditionIdx];

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

const USFCondition* USFParticipant::GetCurrentCondition() const
{
	return Conditions[CurrentConditionIdx];
}

const TArray<USFCondition*> USFParticipant::GetAllConditions() const
{
	return Conditions;
}

FString USFParticipant::GetID() const
{
	return ParticipantID;
}
