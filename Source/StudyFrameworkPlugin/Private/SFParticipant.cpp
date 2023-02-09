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

	StartTime = FPlatformTime::Seconds();

	ParticipantLoggingInfix = "LogParticipant-" + FString::FromInt(ParticipantID) + "_" + FDateTime::Now().ToString();
	FSFLoggingUtils::SetupParticipantLoggingStream(ParticipantLoggingInfix);

	return true;
}

void USFParticipant::SetStudyConditions(TArray<USFCondition*> NewConditions)
{
	Conditions = NewConditions;

	TArray<FString> UniqueNames;
	for(USFCondition* Condition : Conditions)
	{
		if(UniqueNames.Contains(Condition->UniqueName))
		{
			FSFLoggingUtils::Log("Conditions array contain at least twice a condition with unique name: " + Condition->UniqueName + " Doubling Unique names is problematic!!!", true);
		}
		UniqueNames.Add(Condition->UniqueName);
	}

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

	TArray<TSharedPtr<FJsonValue>> IndependentArray;
	for (auto Entry : IndependentVariablesValues) {
		TSharedPtr<FJsonObject> JsonVar = Entry.Key->GetAsJson();
		JsonVar->SetStringField("Value", Entry.Value);
		IndependentArray.Add(MakeShared<FJsonValueObject>(JsonVar));
	}
	Json->SetArrayField("Independent Variables", IndependentArray);

	FSFUtils::WriteJsonToFile(Json, "StudyRuns/Participant_" + FString::FromInt(ParticipantID) + ".txt");
}

void USFParticipant::UpdateIndependentVarsExecutionJsonFile() const
{
	if (ParticipantID == -1)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			FString::FromInt(ParticipantID) + " is not to be read, probably called on init so everything is fine!", false);
		return;
	}
	TSharedPtr<FJsonObject> Json = FSFUtils::ReadJsonFromFile(
		"StudyRuns/Participant_" + FString::FromInt(ParticipantID) + ".txt");
	if (Json == nullptr)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			FString::FromInt(ParticipantID) + " cannot be read!", true);
		return;
	}

	TArray<TSharedPtr<FJsonValue>> IndependentArray;
	for (auto Entry : IndependentVariablesValues) {
		TSharedPtr<FJsonObject> JsonVar = Entry.Key->GetAsJson();
		JsonVar->SetStringField("Value", Entry.Value);
		IndependentArray.Add(MakeShared<FJsonValueObject>(JsonVar));
	}
	Json->RemoveField("Independent Variables");
	Json->SetArrayField("Independent Variables", IndependentArray);

	FSFUtils::WriteJsonToFile(Json, "StudyRuns/Participant_" + FString::FromInt(ParticipantID) + ".txt");

}

void USFParticipant::ReadExecutionJsonFile(int ParticipantID, TArray<USFCondition*>& Conditions_Out, TMap<USFIndependentVariable*, FString>& IndependentVariablesValues_Out)
{
	if(ParticipantID==-1)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			FString::FromInt(ParticipantID) + " is not to be read, probably called on init so everything is fine!", false);
		return;
	}
	TSharedPtr<FJsonObject> Json = FSFUtils::ReadJsonFromFile(
		"StudyRuns/Participant_" + FString::FromInt(ParticipantID) + ".txt");
	TArray<USFCondition*> LoadedConditions;
	if (Json == nullptr)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			FString::FromInt(ParticipantID) + " cannot be read!", true);
		return;
	}

	TArray<TSharedPtr<FJsonValue>> ConditionsArray = Json->GetArrayField("Conditions");
	for (TSharedPtr<FJsonValue> ConditionJson : ConditionsArray)
	{
		USFCondition* Condition = NewObject<USFCondition>();
		Condition->FromJson(ConditionJson->AsObject());
		LoadedConditions.Add(Condition);
	}
	Conditions_Out = LoadedConditions;

	IndependentVariablesValues_Out.Empty();
	TArray<TSharedPtr<FJsonValue>> IndependentArray = Json->GetArrayField("Independent Variables");
	for (TSharedPtr<FJsonValue> VarJson : IndependentArray) {
		USFIndependentVariable* Var = NewObject<USFIndependentVariable>();
		Var->FromJson(VarJson->AsObject());
		FString Val = VarJson->AsObject()->GetStringField("Value");
		IndependentVariablesValues_Out.Add(Var, Val);
	}

}

FString USFParticipant::GetCurrentTimeAsString() const
{
	return FString::Printf(TEXT("%.3f"), GetCurrentTime());
}

float USFParticipant::GetCurrentTime() const
{
	return FPlatformTime::Seconds() - StartTime;
}

void USFParticipant::StoreInPhaseLongTable() const
{
	USFCondition* CurrCondition = GetCurrentCondition();

	FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/Phase_" + CurrCondition->PhaseName + ".csv";

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

void USFParticipant::StoreInIndependentVarLongTable() const
{
	FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/IndependentVariables.csv";
	
	if (!FPaths::FileExists(Filename)) {
		FString Header = "ParticipantID";
		for (auto Var : IndependentVariablesValues) {
			Header += "," + Var.Key->Name;
		}
		Header += "\n";
		FFileHelper::SaveStringToFile(*Header, *Filename);
	}

	FString VarValues = FString::FromInt(ParticipantID);
	for (auto Var : IndependentVariablesValues) {
		VarValues += "," + Var.Value;
	}

	FString StartStringSearch = FString::FromInt(ParticipantID) + ",";
	TArray<FString> ExistingLines;
	FFileHelper::LoadFileToStringArray(ExistingLines, *Filename);
	int found = -1;
	for (int i = 0; i < ExistingLines.Num(); i++) {
		if (ExistingLines[i].StartsWith(StartStringSearch)) {
			found = i;
			break;
		}
	}
	if (found != -1) {
		ExistingLines[found] = VarValues;
		FString ToSave = "";
		for (int i = 0; i < ExistingLines.Num(); i++) {
			ToSave += ExistingLines[i] + "\n";
		}
		FFileHelper::SaveStringToFile(ToSave, *Filename);
	}
	else {
		VarValues += "\n";
		FFileHelper::SaveStringToFile(*VarValues, *Filename, FFileHelper::EEncodingOptions::AutoDetect,
			&IFileManager::Get(), EFileWrite::FILEWRITE_Append);
	}
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
	//save also inpdependent variables if something is configured during the study
	StoreInIndependentVarLongTable();
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
	TArray<USFCondition*> LoadedConditions;
	TMap<USFIndependentVariable*, FString> LoadedIndependentVariablesValues;
	ReadExecutionJsonFile(GetLastParticipantId(), LoadedConditions, LoadedIndependentVariablesValues);
	return LoadedConditions;
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

void USFParticipant::LoadLastParticipantsIndependentVariables()
{
	TArray<USFCondition*> LoadedConditions;
	TMap<USFIndependentVariable*, FString> LoadedIndependentVariablesValues;
	ReadExecutionJsonFile(GetLastParticipantId(), LoadedConditions, LoadedIndependentVariablesValues);
	IndependentVariablesValues = LoadedIndependentVariablesValues;
}

const FString& USFParticipant::GetParticipantLoggingInfix() const
{
	return ParticipantLoggingInfix;
}

const TPair<USFIndependentVariable*, FString> USFParticipant::GetIndependentVariable(const FString& VarName)
{
	for (auto Elem : IndependentVariablesValues) {
		if (Elem.Key->Name == VarName) {
			return Elem;
		}
	}
	return TPair<USFIndependentVariable*, FString>(nullptr, "");
}

void USFParticipant::SetIndependentVariableValue(const FString& VarName, const FString& Value) {
	bool updated = false;
	for (auto& Elem : IndependentVariablesValues) {
		if (Elem.Key->Name == VarName) {
			Elem.Value = Value;
			updated = true;
		}
	}
	if (updated) {
		USFLoggingBPLibrary::LogComment("Recorded new value for independent variable " + VarName + ": " + Value);
		UpdateIndependentVarsExecutionJsonFile();
		StoreInIndependentVarLongTable();
	}
}

void USFParticipant::SetIndependentVariablesFromStudySetup(ASFStudySetup* Setup)
{
	IndependentVariablesValues.Empty();
	for (auto Var : Setup->IndependentVariables) {
		FString Value = "";
		if (Var->bAskedAtBeginning) {
			switch (Var->ValueType) {

			case EValType::TEXT:
			{
				FString Answer;
				int Result = FSFUtils::OpenCustomDialogText(Var->Name, Var->Prompt, "", Answer);
				if (Result < 0) {
					FSFLoggingUtils::Log("[USFParticipant::SetIndependentVariablesFromStudySetup] The window for the variable was closed without giving an answer!", false);
				}
				else {
					Value = Answer;
				}
			}
			break;

			case EValType::MULTIPLECHOICE:
			{
				int Answer = FSFUtils::OpenCustomDialog(Var->Name, Var->Prompt, Var->Options);
				if (Answer < 0) {
					FSFLoggingUtils::Log("[USFParticipant::SetIndependentVariablesFromStudySetup] The window for the variable was closed without selecting anything!", false);
				}
				else {
					Value = Var->Options[Answer];
				}
			}
			break;

			}
		}
		IndependentVariablesValues.Add(DuplicateObject(Var, this), Value);
		USFLoggingBPLibrary::LogComment("Recorded value for independent variable " + Var->Name + ": " + Value);
	}
}

bool USFParticipant::LoadConditionsFromJson()
{
	if (ParticipantID == -1)
	{
		FSFLoggingUtils::Log("[USFParticipant::LoadConditionsFromJson] ParticipantID == -1, maybe nothing stored?", true);
		return false;
	}

	TArray<USFCondition*> LoadedConditions;
	TMap<USFIndependentVariable*, FString> LoadedIndependentVariablesValues;
	ReadExecutionJsonFile(GetLastParticipantId(), LoadedConditions, LoadedIndependentVariablesValues);
	Conditions = LoadedConditions;

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
			FSFLoggingUtils::Log("[USFParticipant::RecoverStudyResultsOfFinishedConditions] Cannot read file: " + Filename, false);
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
