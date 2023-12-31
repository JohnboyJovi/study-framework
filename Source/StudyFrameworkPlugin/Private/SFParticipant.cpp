// Fill out your copyright notice in the Description page of Project Settings.


#include "SFParticipant.h"


#include "IUniversalLogging.h"
#include "SFGameInstance.h"
#include "HAL/FileManagerGeneric.h"
#include "Help/SFUtils.h"
#include "Logging/SFLoggingBPLibrary.h"
#include "Logging/SFLoggingUtils.h"
#include "JsonUtilities.h"

USFParticipant::USFParticipant()
{
}

USFParticipant::~USFParticipant()
{
}

bool USFParticipant::Initialize(int SequenceNumber, FString ID)
{
	ParticipantSequenceNumber = SequenceNumber;
	ParticipantID = ID;

	StartTime = FPlatformTime::Seconds();

	ParticipantLoggingInfix = "LogParticipant-" + ParticipantID + "_" + FDateTime::Now().ToString();
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

	Json->SetStringField("ParticipantID", ParticipantID);
	Json->SetNumberField("ParticipantSequenceNumber", ParticipantSequenceNumber);

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

	FSFUtils::WriteJsonToFile(Json, "StudyRuns/Participant_" + ParticipantID + ".txt");
}

void USFParticipant::UpdateIndependentVarsExecutionJsonFile() const
{
	if (ParticipantSequenceNumber == -1)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant with sequence number " +
			FString::FromInt(ParticipantSequenceNumber) + " is not to be read, probably called on init so everything is fine!", false);
		return;
	}
	TSharedPtr<FJsonObject> Json = FSFUtils::ReadJsonFromFile(
		"StudyRuns/Participant_" + ParticipantID + ".txt");
	if (Json == nullptr)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			ParticipantID + " cannot be read!", true);
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

	FSFUtils::WriteJsonToFile(Json, "StudyRuns/Participant_" + ParticipantID + ".txt");

}

void USFParticipant::ReadExecutionJsonFile(FString ParticipantID, TArray<USFCondition*>& Conditions_Out, TMap<USFIndependentVariable*, FString>& IndependentVariablesValues_Out)
{
	if(ParticipantID.IsEmpty())
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			ParticipantID + " is not to be read, probably called on init so everything is fine!", false);
		return;
	}
	TSharedPtr<FJsonObject> Json = FSFUtils::ReadJsonFromFile(
		"StudyRuns/Participant_" + ParticipantID + ".txt");
	TArray<USFCondition*> LoadedConditions;
	if (Json == nullptr)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::ReadExecutionJsonFile] participant json file for participant " +
			ParticipantID + " cannot be read!", true);
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
		for(auto IV : IndependentVariablesValues)
		{
			Header += "," + IV.Key->Name;
		}
		Header += ",Phase";
		for (auto Factor : CurrCondition->FactorLevels)
		{
			Header += "," + Factor.Key;
		}
		for (auto Var : CurrCondition->DependentVariables)
		{
			if(Cast<USFMultipleTrialDependentVariable>(Var))
			{
				//those are stored in their own files
				continue;
			}
			Header += "," + Var->Name;
		}
		Header += ",Time\n";
		FFileHelper::SaveStringToFile(*Header, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	FString ConditionResults = ParticipantID; 
	for (auto IV : IndependentVariablesValues)
	{
		ConditionResults += "," + IV.Value;
	}
	ConditionResults += "," + CurrCondition->PhaseName;
	for (auto Factor : CurrCondition->FactorLevels)
	{
		ConditionResults += "," + Factor.Value;
	}
	for (auto Var : CurrCondition->DependentVariables)
	{
		if (Cast<USFMultipleTrialDependentVariable>(Var))
		{
			//those are stored in their own files
			continue;
		}
		ConditionResults += "," + Var->Value;
	}
	ConditionResults += "," + FString::Printf(TEXT("%.2f"), CurrCondition->GetTimeTaken());
	//append this
	ConditionResults += "\n";
	FFileHelper::SaveStringToFile(*ConditionResults, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
	                              &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}

void USFParticipant::StoreTrialInTrialDVLongTable(USFMultipleTrialDependentVariable* DependentVariable, TArray<FString> Values) const
{
	USFCondition* CurrCondition = GetCurrentCondition();
	FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/Phase_" + CurrCondition->PhaseName + "_" + DependentVariable->Name + ".csv";

	if (!FPaths::FileExists(Filename))
	{
		FString Header = "ParticipantID";
		for (auto IV : IndependentVariablesValues)
		{
			Header += "," + IV.Key->Name;
		}
		Header += ",Phase";
		for (auto Factor : CurrCondition->FactorLevels)
		{
			Header += "," + Factor.Key;
		}
		Header += ",Trial";
		for (const FString& SubName : DependentVariable->SubVariableNames)
		{
			Header += "," + SubName;
		}
		Header += "\n";
		FFileHelper::SaveStringToFile(*Header, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	FString TrialResult = ParticipantID;
	for (auto IV : IndependentVariablesValues)
	{
		TrialResult += "," + IV.Value;
	}
	TrialResult += "," + CurrCondition->PhaseName;
	for (auto Factor : CurrCondition->FactorLevels)
	{
		TrialResult += "," + Factor.Value;
	}
	TrialResult += "," + FString::FromInt(DependentVariable->Values.size());
	for (const FString& Value : Values)
	{
		TrialResult += "," + Value;
	}

	//append this
	TrialResult += "\n";
	FFileHelper::SaveStringToFile(*TrialResult, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
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
		FFileHelper::SaveStringToFile(*Header, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	FString VarValues = ParticipantID;
	for (auto Var : IndependentVariablesValues) {
		VarValues += "," + Var.Value;
	}

	FString StartStringSearch = ParticipantID + ",";
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
		FFileHelper::SaveStringToFile(ToSave, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
	else {
		VarValues += "\n";
		FFileHelper::SaveStringToFile(*VarValues, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
			&IFileManager::Get(), EFileWrite::FILEWRITE_Append);
	}
}

void USFParticipant::DeleteStoredDataForConditionFromLongTable(USFCondition* Condition)
{
	const FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/Phase_" + Condition->PhaseName + ".csv";
	RemoveLinesOfConditionAndWriteToFile(Condition, Filename);
}

void USFParticipant::DeleteStoredTrialDataForCondition(USFCondition* Condition, USFMultipleTrialDependentVariable* DependentVariable)
{
	const FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/Phase_" + Condition->PhaseName + "_" + DependentVariable->Name + ".csv";
	RemoveLinesOfConditionAndWriteToFile(Condition, Filename);
}

void USFParticipant::RemoveLinesOfConditionAndWriteToFile(USFCondition* Condition, const FString Filename)
{
	TArray<FString> Lines;
	TArray<FString> CleanedLines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *Filename))
	{
		FSFLoggingUtils::Log("[USFParticipant::RemoveLinesOfConditionAndWriteToFile] Cannot read file: " + Filename + " (probably there was no data recorded yet).", false);
		return;
	}

	TArray<FString> HeaderEntries;
	if (Lines.Num() > 0)
	{
		Lines[0].ParseIntoArray(HeaderEntries, TEXT(","), false);
	}
	CleanedLines.Add(Lines[0]);

	bool bHasRemovedLines = false;
	for (int i = 1; i < Lines.Num(); ++i)
	{
		TArray<FString> Entries;
		Lines[i].ParseIntoArray(Entries, TEXT(","), false);		

		if (Entries.Num() > 0 && Entries[0] == ParticipantID)
		{
			//so this is a line of this participant, but also from the right condition?
			bool bRightCondition = true;
			for (auto FactorLevel : Condition->FactorLevels)
			{
				if (Entries[HeaderEntries.Find(FactorLevel.Key)] != FactorLevel.Value)
				{
					//This line does not belong to this condition
					bRightCondition = false;
				}
			}
			if (!bRightCondition)
			{
				CleanedLines.Add(Lines[i]);
			}
			//At least this line will be removed
			else if(!bHasRemovedLines)
			{
				bHasRemovedLines = true;
			}
		}
		else
		{
			//from another participant so just keep this line!
			CleanedLines.Add(Lines[i]);
		}
	}

	if(bHasRemovedLines)
	{
		CreateLongTableBackUp(Filename);
	}

	FFileHelper::SaveStringArrayToFile(CleanedLines, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
		&IFileManager::Get(), EFileWrite::FILEWRITE_None);
}

bool USFParticipant::StartStudy()
{
	// Set first condition
	CurrentConditionIdx = -1;

	USFLoggingBPLibrary::LogComment("Start Study for ParticipantID: " + ParticipantID);

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

int USFParticipant::GetSequenceNumber() const
{
	return ParticipantSequenceNumber;
}

FString USFParticipant::GetID() const
{
	return ParticipantID;
}

bool USFParticipant::WasParticipantIdAlreadyUsed(FString NewParticipantID)
{
	//we highjack the independent variable file for this, since each participant adds entries to it
	FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/IndependentVariables.csv";

	if (!FPaths::FileExists(Filename)) {
		FSFLoggingUtils::Log("[USFParticipant::WasParticipantIdAlreadyUsed] IndependentVariables.csv does not exist (yet?)", false);
		return false;
	}

	FString StartStringSearch = NewParticipantID + ",";
	TArray<FString> ExistingLines;
	FFileHelper::LoadFileToStringArray(ExistingLines, *Filename);
	for (const FString& Line : ExistingLines) {
		if (Line.StartsWith(StartStringSearch)) {
			return true;
		}
	}
	return false;
}

TArray<USFCondition*> USFParticipant::GetLastParticipantsConditions()
{
	TArray<USFCondition*> LoadedConditions;
	TMap<USFIndependentVariable*, FString> LoadedIndependentVariablesValues;
	ReadExecutionJsonFile(GetLastParticipantID(), LoadedConditions, LoadedIndependentVariablesValues);
	//Load WasStarted-Values from JSON, to ensure data is not overwritten without backups, if conditions are restarted
	TMap<FString, bool> LastParticipantHasStartedConditionValues = GetLastParticipantHasStartedConditionValues();
	for(int i = 0; i<LoadedConditions.Num(); i++)
	{
		if(LastParticipantHasStartedConditionValues.Contains(LoadedConditions[i]->CreateIdentifiableName()))
		{
			LoadedConditions[i]->SetbStarted(LastParticipantHasStartedConditionValues[LoadedConditions[i]->CreateIdentifiableName()]);
		}
	}
	return LoadedConditions;
}

int USFParticipant::GetLastParticipantSequenceNumber()
{
	TSharedPtr<FJsonObject> ParticipantJson = FSFUtils::ReadJsonFromFile("StudyRuns/LastParticipant.txt");
	if (ParticipantJson == nullptr)
	{
		//file does not exist or something else went wrong
		return -1;
	}
	return ParticipantJson->GetNumberField("ParticipantSequenceNumber");
}

FString USFParticipant::GetLastParticipantID()
{
	TSharedPtr<FJsonObject> ParticipantJson = FSFUtils::ReadJsonFromFile("StudyRuns/LastParticipant.txt");
	if (ParticipantJson == nullptr)
	{
		//file does not exist or something else went wrong
		return "";
	}
	return ParticipantJson->GetStringField("ParticipantID");
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

TMap<FString, bool> USFParticipant::GetLastParticipantHasStartedConditionValues()
{
	TMap<FString, bool> ConditionsStarted;
	TSharedPtr<FJsonObject> ParticipantJson = FSFUtils::ReadJsonFromFile("StudyRuns/LastParticipant.txt");
	if (ParticipantJson != nullptr)
	{
		TArray<TSharedPtr<FJsonValue>> StartedConditionsArray = ParticipantJson->GetArrayField("StartedConditions");
		for(auto& JsonValue : StartedConditionsArray)
		{
			TSharedPtr<FJsonObject> Entry = JsonValue->AsObject();
			if(Entry.IsValid())
			{
				ConditionsStarted.Add(Entry->GetStringField("Condition"), Entry->GetBoolField("WasStarted"));
			}			
		}

	}
	return ConditionsStarted;

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
	ReadExecutionJsonFile(GetLastParticipantID(), LoadedConditions, LoadedIndependentVariablesValues);
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
			USFLoggingBPLibrary::LogComment("Recorded value for independent variable " + Var->Name + ": " + Value);
		}
		IndependentVariablesValues.Add(DuplicateObject(Var, this), Value);
	}
}

bool USFParticipant::LoadConditionsFromJson()
{
	TArray<USFCondition*> LoadedConditions;
	TMap<USFIndependentVariable*, FString> LoadedIndependentVariablesValues;
	ReadExecutionJsonFile(GetLastParticipantID(), LoadedConditions, LoadedIndependentVariablesValues);
	Conditions = LoadedConditions;

	if (Conditions.Num() == 0)
	{
		FSFLoggingUtils::Log(
			"[USFParticipant::LoadContitionsFromJson] No Conditions could be loaded for Participant " +
			ParticipantID, true);
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
		if (!FFileHelper::LoadFileToStringArray(Lines, *Filename))
		{
			FSFLoggingUtils::Log("[USFParticipant::RecoverStudyResultsOfFinishedConditions] Cannot read file: " + Filename + " (probably there was no data recorded yet).", false);
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

			if(Entries.Num()>0 && Entries[0] == ParticipantID)
			{
				Condition->RecoverStudyResults(HeaderEntries, Entries);
			}
		}

		//now also recover SFMultipleTrialDependentVariable data:
		for(USFDependentVariable* Var : Condition->DependentVariables)
		{
			if(auto MultiTrialVar = Cast<USFMultipleTrialDependentVariable>(Var))
			{
				MultiTrialVar->RecoverStudyResults(Condition, ParticipantID);
			}
		}
	}
}

void USFParticipant::CreateLongTableBackUp(const FString PathToSrcFile) const
{	
	IFileManager& FileManager = IFileManager::Get();
	const FString ReplaceWith = "StudyFramework/StudyLogs/RecyclingBin/" + CurrentBackUpFolderName + "/";
	const FString PathToDestFile = PathToSrcFile.Replace(TEXT("StudyFramework/StudyLogs/"), *ReplaceWith);
	FileManager.Copy( *PathToDestFile, *PathToSrcFile);
	FSFLoggingUtils::Log("Created Backup: " + PathToDestFile);

}

void USFParticipant::ClearPhaseLongtables(ASFStudySetup* StudySetup)
{
	const FString LongTableFolder = FPaths::ProjectDir() + "StudyFramework/StudyLogs/";
	const FString Extension = "*.csv";
	const FString SearchPattern = LongTableFolder + Extension;
	TArray<FString> FileNames;
	IFileManager& FileManager = IFileManager::Get();
	FileManager.FindFiles(FileNames, *SearchPattern, true, false);
	//Instead of actually deleting files, we want to move them to a recycling bin folder, to minimize risk of data loss
	FString NewParentFolderPath = LongTableFolder + "RecyclingBin/" + "RestartStudyBackup-" + FDateTime::Now().ToString() + "/";
	for (FString Filename : FileNames)
	{
		const FString FullName = LongTableFolder + Filename;
		FileManager.Move(*(NewParentFolderPath + Filename), *FullName);
	}
	FSFLoggingUtils::Log("Moved .csv files: " + NewParentFolderPath);
}

void USFParticipant::SetCurrentBackUpFolderName(FString BackUpFolderName)
{
	CurrentBackUpFolderName = BackUpFolderName;
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
		else if (!NextCondition->WasStarted()) //otherwise we are restarting a condition, so obviously not finishing the current one
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

	Json->SetNumberField("ParticipantSequenceNumber", ParticipantSequenceNumber);
	Json->SetStringField("ParticipantID", ParticipantID);
	bool bFinished = true;
	TArray<TSharedPtr<FJsonValue>> StartedConditions; //Cannot use TMap because we want to save to JSON
	for (USFCondition* Condition : Conditions)
	{
		bFinished = bFinished && (Condition->IsFinished() || !Condition->HasRequiredVariables());
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetStringField("Condition", Condition->CreateIdentifiableName());
		JsonObject->SetBoolField("WasStarted", Condition->WasStarted());
		StartedConditions.Add(MakeShared<FJsonValueObject>(JsonObject));
	}
	Json->SetArrayField("StartedConditions", StartedConditions);
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
