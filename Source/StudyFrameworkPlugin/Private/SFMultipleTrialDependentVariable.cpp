#include "SFMultipleTrialDependentVariable.h"

#include "Dom/JsonObject.h"
#include "Help/SFUtils.h"
#include "Logging/SFLoggingUtils.h"


USFMultipleTrialDependentVariable::USFMultipleTrialDependentVariable()
{
}

TSharedPtr<FJsonObject> USFMultipleTrialDependentVariable::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = Super::GetAsJson();
	TArray<TSharedPtr<FJsonValue>> SubNamesArray;
	for(FString SubName : SubVariableNames)
	{
		TSharedRef<FJsonValue> JsonValue = MakeShared<FJsonValueString>(SubName);
		SubNamesArray.Add(JsonValue);
	}
	Json->SetArrayField("SubVariableNames", SubNamesArray);
	Json->SetStringField("Type", "USFMultipleTrialDependentVariable");

	return Json;
}

void USFMultipleTrialDependentVariable::FromJsonInternal(TSharedPtr<FJsonObject> Json)
{
	Super::FromJsonInternal(Json);
	SubVariableNames.Empty();
	for(TSharedPtr<FJsonValue> SubNameJson : Json->GetArrayField("SubVariableNames"))
	{
		SubVariableNames.Add(SubNameJson->AsString());
	}
}

void USFMultipleTrialDependentVariable::RecoverStudyResults(USFCondition* Condition, FString ParticipantID)
{
	const FString Filename = FPaths::ProjectDir() + "StudyFramework/StudyLogs/Phase_" + Condition->PhaseName + "_" + Name + ".csv";
	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *Filename))
	{
		FSFLoggingUtils::Log("[USFMultipleTrialDependentVariable::RecoverStudyResults] Cannot read file: " + Filename + " (probably there was no data recorded yet).", false);
	}

	TArray<FString> HeaderEntries;
	if (Lines.Num() > 0)
	{
		Lines[0].ParseIntoArray(HeaderEntries, TEXT(","), false);
	}

	for (int i = 1; i < Lines.Num(); ++i)
	{
		TArray<FString> Entries;
		Lines[i].ParseIntoArray(Entries, TEXT(","), false);

		if (Entries.Num() > 0 && Entries[0] == ParticipantID)
		{
			RecoverStudyResultsOfLine(HeaderEntries, Entries, Condition);
		}
	}
}

void USFMultipleTrialDependentVariable::RecoverStudyResultsOfLine(const TArray<FString>& Header,
	const TArray<FString>& Entries, USFCondition* Condition)
{
	check(Header.Num() == Entries.Num())

	for (auto FactorLevel : Condition->FactorLevels)
	{
		if (!Header.Contains(FactorLevel.Key) || Entries[Header.Find(FactorLevel.Key)] != FactorLevel.Value)
		{
			//This line does not belong to this condition
			return;;
		}
	}

	//so this is the right condition

	//these should be in the right order, just double-check
	check(FCString::Atoi(*Entries[Header.Find("Trial")]) == Values.size())

	std::vector<FString> Data;
	for (const FString& SubName : SubVariableNames)
	{
		Data.push_back( Entries[Header.Find(SubName)] );
	}
	Values.push_back(Data);
}

