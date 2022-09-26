// Fill out your copyright notice in the Description page of Project Settings.


#include "SFParticipant.h"


#include "IUniversalLogging.h"
#include "SFGameInstance.h"
#include "Help/SFUtils.h"
#include "Logging/SFLoggingBPLibrary.h"
#include "Logging/SFLoggingUtils.h"

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
	ILogStream* ParticipantLog = UniLog.NewLogStream("ParticipantLog", "StudyFramework/StudyLogs/ParticipantLogs",
	                                                 Filename, false);
	ILogStream* PositionLog = UniLog.NewLogStream("PositionLog", "StudyFramework/StudyLogs/PositionLogs",
		"Position"+Filename, false);
	if (USFGameInstance::Get())
	{
		USFGameInstance::Get()->GetLogObject()->LogHeaderRows();
	}
	else
	{
		FSFLoggingUtils::Log("GameInstance not set up yet, no header rows are written to participant logs.");
	}
	return true;
}

void USFParticipant::SetStudyConditions(TArray<USFCondition*> NewConditions)
{
	Conditions = NewConditions;

	// Create initial Json file
	GenerateExecutionJsonFile();
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
	if(ParticipantID==-1)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			FString::FromInt(ParticipantID) + " is not to be read, probably called on init so everything is fine!", false);
		return {};
	}
	TSharedPtr<FJsonObject> Json = FSFUtils::ReadJsonFromFile(
		"StudyRuns/Participant_" + FString::FromInt(ParticipantID) + ".txt");
	TArray<USFCondition*> LoadedConditions;
	if (Json == nullptr)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			FString::FromInt(ParticipantID) + " cannot be read!", true);
		return {};
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

	FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/Phase_" + CurrCondition->PhaseName + ".csv";

	if (!FPaths::FileExists(Filename))
	{
		FString Header = "ParticipantID";
		Header += ",Map";
		for (auto Factor : CurrCondition->FactorLevels)
		{
			Header += "," + Factor.Key;
		}
		for (auto Var : CurrCondition->DependentVariablesValues)
		{
			Header += "," + Var.Key->Name;
		}
		Header += ",Time\n";
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
	ConditionResults += "," + FString::Printf(TEXT("%.2f"), CurrCondition->GetTimeTaken());
	//append this
	ConditionResults += "\n";
	FFileHelper::SaveStringToFile(*ConditionResults, *Filename, FFileHelper::EEncodingOptions::AutoDetect,
	                              &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}

bool USFParticipant::StartStudy()
{
	// Set first condition
	CurrentConditionIdx = -1;

	USFLoggingBPLibrary::LogComment("Start Study for ParticipantID: " + FString::FromInt(ParticipantID));

	return true;
}

void USFParticipant::EndStudy()
{
	USFLoggingBPLibrary::LogComment("EndStudy");
	LogCurrentParticipant();
	StoreInPhaseLongTable();
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
		FSFLoggingUtils::Log("[USFParticipant::NextCondition()]: All conditions already ran, no NextCondition", false);
		return nullptr;
	}
	USFCondition* UpcomingCondition = Conditions[CurrentConditionIdx + 1];
	return UpcomingCondition;
}

const TArray<USFCondition*> USFParticipant::GetAllConditions() const
{
	return Conditions;
}

int USFParticipant::GetCurrentConditionNumber() const
{
	return CurrentConditionIdx;
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
	TSharedPtr<FJsonObject> ParticipantJson = FSFUtils::ReadJsonFromFile("StudyRuns/LastParticipant.txt");
	if (ParticipantJson == nullptr)
	{
		//file does not exist or something else went wrong
		return -1;
	}
	return ParticipantJson->GetNumberField("ParticipantID");
}

int USFParticipant::GetLastParticipantLastConditionStarted()
{
	TSharedPtr<FJsonObject> ParticipantJson = FSFUtils::ReadJsonFromFile("StudyRuns/LastParticipant.txt");
	if (ParticipantJson == nullptr)
	{
		//file does not exist or something else went wrong
		return -1;
	}
	return ParticipantJson->GetNumberField("CurrentConditionIdx");
}

bool USFParticipant::GetLastParticipantFinished()
{
	TSharedPtr<FJsonObject> ParticipantJson = FSFUtils::ReadJsonFromFile("StudyRuns/LastParticipant.txt");
	if (ParticipantJson == nullptr)
	{
		//file does not exist or something else went wrong
		return true;
	}
	return ParticipantJson->GetBoolField("Finished");
}

ASFStudySetup* USFParticipant::GetLastParticipantSetup()
{
	TSharedPtr<FJsonObject> ParticipantJson = FSFUtils::ReadJsonFromFile("StudyRuns/LastParticipant.txt");
	if (ParticipantJson == nullptr || !ParticipantJson->HasField("StudySetup"))
	{
		//file does not exist or something else went wrong
		return nullptr;
	}
	FString SetupFile = ParticipantJson->GetStringField("StudySetup");
	ASFStudySetup* Setup = NewObject<ASFStudySetup>();
	Setup->JsonFile = SetupFile;
	Setup->LoadFromJson();

	return Setup;
}

bool USFParticipant::LoadConditionsFromJson()
{
	if (ParticipantID == -1)
	{
		FSFLoggingUtils::Log("[USFParticipant::LoadConditionsFromJson] ParticipantID == -1, maybe nothing stored?", true);
		return false;
	}

	Conditions = ReadExecutionJsonFile(ParticipantID);

	if (Conditions.Num() == 0)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::LoadContitionsFromJson] No Conditions could be loaded for Participant " +
			FString::FromInt(ParticipantID), true);
		return false;
	}
	return true;
}

void USFParticipant::RecoverStudyResultsOfFinishedConditions()
{
	//this is not the most effective way of recovering but the most trivial (long tables will be read multiple times)
	for (USFCondition* Condition : Conditions)
	{
		const FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/Phase_" + Condition->PhaseName + ".csv";
		TArray<FString> Lines;
		if (!FFileHelper::LoadANSITextFileToStrings(*Filename, nullptr, Lines))
		{
			FSFLoggingUtils::Log("[USFParticipant::RecoverStudyResultsOfFinishedConditions] Cannot read file: " + Filename, true);
		}

		TArray<FString> HeaderEntries;
		if(Lines.Num()>0)
		{
			Lines[0].ParseIntoArray(HeaderEntries,TEXT(","), false);
		}

		for (int i=1; i<Lines.Num(); ++i)
		{
			TArray<FString> Entries;
			Lines[i].ParseIntoArray(Entries,TEXT(","), false);

			if(Entries.Num()>0 && FCString::Atoi(*Entries[0]) == ParticipantID)
			{
				Condition->RecoverStudyResults(HeaderEntries, Entries);
			}
		}
	}
}

void USFParticipant::ClearPhaseLongtables(ASFStudySetup* StudySetup)
{
	for (int i = 0; i < StudySetup->GetNumberOfPhases(); ++i)
	{
		const FString PhaseName = StudySetup->GetPhase(i)->PhaseName;
		const FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/Phase_" + PhaseName + ".csv";
		if (FPaths::FileExists(Filename))
		{
			IFileManager& FileManager = IFileManager::Get();
			FileManager.Delete(*Filename);
		}
	}
}

bool USFParticipant::SetCondition(const USFCondition* NextCondition)
{
	if (!NextCondition)
		return false;

	if (GetCurrentCondition() && GetCurrentCondition()->WasStarted())
	{
		//we already ran a condition so store it
		if (GetCurrentCondition()->IsFinished())
		{
			StoreInPhaseLongTable();
		}
		else
		{
			FSFLoggingUtils::Log(
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
	FSFLoggingUtils::Log("[USFParticipant::SetCondition()]: Requested condition is not one of my conditions!", true);
	return false;
}

void USFParticipant::LogCurrentParticipant() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

	Json->SetNumberField("ParticipantID", ParticipantID);
	bool bFinished = true;
	for (USFCondition* Condition : Conditions)
	{
		bFinished = bFinished && (Condition->IsFinished() || !Condition->HasRequiredVariables());
	}
	Json->SetBoolField("Finished", bFinished);
	Json->SetNumberField("CurrentConditionIdx", CurrentConditionIdx);
	if (USFGameInstance::Get() && USFGameInstance::Get()->GetStudySetup())
	{
		Json->SetStringField("StudySetup", USFGameInstance::Get()->GetStudySetup()->JsonFile);
	}
	else
	{
		FSFLoggingUtils::Log("[USFParticipant::LogCurrentParticipant] StudySetup not accessible!", true);
	}


	FSFUtils::WriteJsonToFile(Json, "StudyRuns/LastParticipant.txt");
}
