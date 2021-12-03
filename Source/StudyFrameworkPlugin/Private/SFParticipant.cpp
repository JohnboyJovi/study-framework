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


bool USFParticipant::Initialize(int Participant, FString JsonFilePath, FString LogName, FString SaveDataLogName)
{
	ParticipantID = Participant;

	// TODO initialize Logger!!!   or not???
	Logger = NewObject<USFLogger>();
	Logger->Initialize(this, JsonFilePath, LogName, SaveDataLogName);

	return true;
}

void USFParticipant::GenerateExecutionJsonFile() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

	Json->SetNumberField("ParticipantID", ParticipantID);

	TArray<TSharedPtr<FJsonValue>> ConditionsArray;
	for (auto Condition : Conditions)
	{
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Condition->GetAsJson());
		ConditionsArray.Add(JsonValue);
	}
	Json->SetArrayField("Conditions", ConditionsArray);

	FSFUtils::WriteJsonToFile(Json, "StudyRuns/Participant_" + FString::FromInt(ParticipantID) + ".txt");
}

TArray<USFCondition*> USFParticipant::ReadExecutionJsonFile(int ParticipantID)
{
	TSharedPtr<FJsonObject> Json = FSFUtils::ReadJsonFromFile(
		"StudyRuns/Participant_" + FString::FromInt(ParticipantID) + ".txt");
	TArray<USFCondition*> LoadedConditions;
	if (Json == nullptr)
	{
		FSFUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			FString::FromInt(ParticipantID) + " cannot be read!", true);
		return LoadedConditions;
	}

	TArray<TSharedPtr<FJsonValue>> ConditionsArray = Json->GetArrayField("Conditions");
	for (TSharedPtr<FJsonValue> ConditionJson : ConditionsArray)
	{
		USFCondition* Condition = NewObject<USFCondition>();
		Condition->FromJson(ConditionJson->AsObject());
		LoadedConditions.Add(Condition);
	}
	return LoadedConditions;
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
		" conditions for participant " + FString::FromInt(ParticipantID),
		false);

	// Create initial Json file
	GenerateExecutionJsonFile();

	// Set first condition
	CurrentConditionIdx = -1;

	return true;
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

USFCondition* USFParticipant::GetNextCondition() const
{
	// Get next Condition
	if (CurrentConditionIdx + 1 >= Conditions.Num())
	{
		FSFUtils::Log("[USFParticipant::NextCondition()]: All conditions already ran, no NextCondition", false);
		return nullptr;
	}
	USFCondition* UpcomingCondition = Conditions[CurrentConditionIdx + 1];
	return UpcomingCondition;
}

const TArray<USFCondition*> USFParticipant::GetAllConditions() const
{
	return Conditions;
}

int USFParticipant::GetID() const
{
	return ParticipantID;
}

TArray<USFCondition*> USFParticipant::GetLastParticipantsConditions()
{
	return ReadExecutionJsonFile(GetLastParticipantId());
}

int USFParticipant::GetLastParticipantId()
{
	TSharedPtr<FJsonObject> ParticpantJson = FSFUtils::ReadJsonFromFile("StudyRuns/LastParticipant.txt");
	if (ParticpantJson == nullptr)
	{
		//file does not exist or something else went wrong
		return -1;
	}
	return ParticpantJson->GetNumberField("ParticipantID");
}

bool USFParticipant::LoadConditionsFromJson()
{
	if (ParticipantID == -1)
	{
		FSFUtils::Log("[USFParticipant::LoadContitionsFromJson] ParticipantID == -1, maybe nothing stored?", true);
		return false;
	}

	Conditions = ReadExecutionJsonFile(ParticipantID);

	if (Conditions.Num() == 0)
	{
		FSFUtils::Log(
			"[USFParticipant::LoadContitionsFromJson] No Conditions could be loaded for Participant " +
			FString::FromInt(ParticipantID), true);
		return false;
	}
	return true;
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
			LogCurrentParticipant();
			return true;
		}
	}
	FSFUtils::Log("[USFParticipant::SetCondition()]: Requested condition is not one of my conditions!", true);
	return false;
}

void USFParticipant::LogCurrentParticipant() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

	Json->SetNumberField("ParticipantID", ParticipantID);
	bool bFinished = true;
	for (USFCondition* Condition : Conditions)
	{
		bFinished = bFinished && Condition->bConditionFinished;
	}
	Json->SetBoolField("Finished", bFinished);
	Json->SetNumberField("CurrentConditionIdx", CurrentConditionIdx);


	FSFUtils::WriteJsonToFile(Json, "StudyRuns/LastParticipant.txt");
}
