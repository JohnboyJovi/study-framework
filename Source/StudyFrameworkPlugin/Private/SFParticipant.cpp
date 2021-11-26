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

	// TODO initialize Logger!!!   or not???
	Logger = NewObject<USFLogger>();
	Logger->Initialize(this, JsonFilePath, LogName, SaveDataLogName);

	return true;
}

void USFParticipant::GenerateExecutionJsonFile() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

	Json->SetStringField("ParticipantID", ParticipantID);

	TArray<TSharedPtr<FJsonValue>> ConditionsArray;
	for (auto Condition : Conditions)
	{
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Condition->GetAsJson());
		ConditionsArray.Add(JsonValue);
	}
	Json->SetArrayField("Conditions", ConditionsArray);

	FSFUtils::WriteJsonToFile(Json, "Runs/Participant_" + ParticipantID + ".txt");
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

USFCondition* USFParticipant::NextCondition()
{
	// Get next Condition
	if (CurrentConditionIdx + 1 >= Conditions.Num())
	{
		FSFUtils::Log("[USFParticipant::NextCondition()]: All conditions already ran, EndStudy()", false);
		return nullptr;
	}
	USFCondition* UpcomingCondition = Conditions[CurrentConditionIdx + 1];
	return UpcomingCondition;
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

USFCondition* USFParticipant::GetCurrentCondition() const
{
	if (CurrentConditionIdx >= 0 && CurrentConditionIdx < Conditions.Num())
		return Conditions[CurrentConditionIdx];

	return nullptr;
}

const TArray<USFCondition*> USFParticipant::GetAllConditions() const
{
	return Conditions;
}

FString USFParticipant::GetID() const
{
	return ParticipantID;
}

bool USFParticipant::SetCondition(const USFCondition* NextCondition)
{
	if (!NextCondition)
		return false;

	for (int i = 0; i < Conditions.Num(); ++i)
	{
		if (Conditions[i] == NextCondition)
		{
			CurrentConditionIdx = i;
			return true;
		}
	}
	FSFUtils::Log("[USFParticipant::SetCondition()]: Requested condition is not one of my conditions!", true);
	return false;
}
