// Fill out your copyright notice in the Description page of Project Settings.


#include "SFParticipant.h"


#include "IUniversalLogging.h"
#include "SFGameInstance.h"
#include "Help/SFUtils.h"

USFParticipant::USFParticipant()
{
}

USFParticipant::~USFParticipant()
{
}

bool USFParticipant::Initialize(int Participant)
{
	ParticipantID = Participant;

	
	const FString Timestamp = FDateTime::Now().ToString();
	const FString Filename = "LogParticipant-" + FString::FromInt(ParticipantID) + "_" + Timestamp + ".txt";
	ILogStream* ParticipantLog = UniLog.NewLogStream("ParticipantLog", "Saved/Results",
	                                                 Filename, false);
	StartTime = FPlatformTime::Seconds();

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

FString USFParticipant::GetCurrentTime() const
{
	const double CurrTime = FPlatformTime::Seconds() - StartTime;
	return FString::Printf(TEXT("%.3f"), CurrTime);
}

void USFParticipant::StoreInPhaseLongTable() const
{
	USFCondition* CurrCondition = GetCurrentCondition();

	FString Filename = FPaths::ProjectSavedDir() + "Results/Phase_" + CurrCondition->PhaseName + ".csv";

	if (!FPaths::FileExists(Filename))
	{
		FString Header = "ParticipantID";
		for (auto Factor : CurrCondition->FactorLevels)
		{
			Header += "," + Factor.Key;
		}
		for (auto Var : CurrCondition->DependentVariablesValues)
		{
			Header += "," + Var.Key->Name;
		}
		Header += "\n";
		FFileHelper::SaveStringToFile(*Header, *Filename);
	}

	FString ConditionResults = FString::FromInt(ParticipantID);
	for (auto Factor : CurrCondition->FactorLevels)
	{
		ConditionResults += "," + Factor.Value;
	}
	for (auto Var : CurrCondition->DependentVariablesValues)
	{
		ConditionResults += "," + Var.Value;
	}
	//append this
	ConditionResults += "\n";
	FFileHelper::SaveStringToFile(*ConditionResults, *Filename, FFileHelper::EEncodingOptions::AutoDetect,
	                              &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}

bool USFParticipant::StartStudy(USFStudySetup* StudySetup)
{
	//TODO: recover from crashed run?
	// If reload an already existing study?
	/*if (FindJsonFile())
	{
		FSFUtils::Log("[USFParticipant::StartStudy()]: Json File found. Loading it now..", false);
		return LoadJsonFile();
	}*/

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

	LogComment("Start Study for ParticipantID: "+FString::FromInt(ParticipantID));

	return true;
}

void USFParticipant::EndStudy()
{
	LogComment("EndStudy");
	StoreInPhaseLongTable();
}

void USFParticipant::LogData(const FString& DependentVariableName, const FString& Value)
{
	USFCondition* CurrCondition = GetCurrentCondition();
	if (!CurrCondition->StoreDependetVariableData(DependentVariableName, Value))
	{
		FSFUtils::Log(
			"Cannot log data '" + Value + "' for dependent variable '" + DependentVariableName +
			"' since it does not exist for this condition!", true);
		return;
	}
	LogComment("Recorded " + DependentVariableName + ": " + Value);

	//the data is stored in the phase long table on SetCondition() or EndStudy()
}

void USFParticipant::LogComment(const FString& Comment)
{
	UniLog.Log("#" + GetCurrentTime() + ": " + Comment, "ParticipantLog");
	FSFUtils::Log("Logged Comment: " + Comment);
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

	if (GetCurrentCondition())
	{
		//we already ran a condition so store it
		if (GetCurrentCondition()->IsFinished())
		{
			StoreInPhaseLongTable();
		}
		else
		{
			FSFUtils::Log(
				"[USFParticipant::SetCondition] Not storing unfinished last condition, when going to next. Make sure all required dependent variables received data!",
				true);
		}
	}

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
		bFinished = bFinished && Condition->IsFinished();
	}
	Json->SetBoolField("Finished", bFinished);
	Json->SetNumberField("CurrentConditionIdx", CurrentConditionIdx);


	FSFUtils::WriteJsonToFile(Json, "StudyRuns/LastParticipant.txt");
}
