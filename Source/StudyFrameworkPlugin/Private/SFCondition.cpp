#include "SFCondition.h"

#include "SFGameInstance.h"
#include "UObject/UObjectGlobals.h"

#include "SFMapFactor.h"
#include "SFMultipleTrialDependentVariable.h"
#include "Logging/SFLoggingBPLibrary.h"
#include "Logging/SFLoggingUtils.h"

USFCondition::USFCondition()
{
}

void USFCondition::Generate(const FString& InPhaseName, const TArray<int>& ConditionIndices,
                            const TArray<USFStudyFactor*>& Factors,
                            const TArray<USFDependentVariable*>& DependentVars)
{
	PhaseName = InPhaseName;
	check(ConditionIndices.Num()==Factors.Num())

	for (int i = 0; i < Factors.Num(); ++i)
	{
		USFStudyFactor* Factor = Factors[i];
		FString FactorLevel = Factor->Levels[ConditionIndices[i]];
		if (Factor->IsA(USFMapFactor::StaticClass()))
		{
			Map = FactorLevel;
			//for better readybility strip path!
			FactorLevel = FPaths::GetBaseFilename(FactorLevel);
		}
		FactorLevels.Add(Factor->FactorName, FactorLevel);
	}

	for (USFDependentVariable* Var : DependentVars)
	{
		DependentVariables.Add(DuplicateObject(Var, this));
	}
	UniqueName = CreateIdentifiableName();
}

TSharedPtr<FJsonObject> USFCondition::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

	Json->SetStringField("Name", UniqueName);
	Json->SetStringField("PhaseName", PhaseName);
	Json->SetStringField("Map", Map);

	TSharedPtr<FJsonObject> FactorLevelsJson = MakeShared<FJsonObject>();
	for (auto FactorLevel : FactorLevels)
	{
		FactorLevelsJson->SetStringField(FactorLevel.Key, FactorLevel.Value);
	}
	Json->SetObjectField("FactorLevels", FactorLevelsJson);

	TArray<TSharedPtr<FJsonValue>> DependentVariablesArray;
	for (auto Var : DependentVariables)
	{
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Var->GetAsJson());
		DependentVariablesArray.Add(JsonValue);
	}
	Json->SetArrayField("DependentVariables", DependentVariablesArray);
	return Json;
}

void USFCondition::FromJson(TSharedPtr<FJsonObject> Json)
{
	UniqueName = Json->GetStringField("Name");
	PhaseName = Json->GetStringField("PhaseName");
	Map = Json->GetStringField("Map");

	TSharedPtr<FJsonObject> FactorLevelsJson = Json->GetObjectField("FactorLevels");
	for (auto FactorLevel : FactorLevelsJson->Values)
	{
		FactorLevels.Add(FactorLevel.Key, FactorLevel.Value->AsString());
	}

	TArray<TSharedPtr<FJsonValue>> DependentVariablesArray = Json->GetArrayField("DependentVariables");
	for (auto Var : DependentVariablesArray)
	{
		USFDependentVariable* DependentVariable = USFDependentVariable::FromJson(Var->AsObject(), this);
		DependentVariables.Add(DependentVariable);
	}
}

FString USFCondition::CreateIdentifiableName()
{
	return ToString();
}

FString USFCondition::ToString() const
{
	FString Out = PhaseName;
	for (auto Level : FactorLevels)
	{
		Out += "_" + Level.Value;
	}
	return Out;
}

bool USFCondition::operator==(USFCondition& Other)
{
	//this should work, since we use CreateIdentifiableName() for the Name
	return UniqueName == Other.UniqueName;
}

bool USFCondition::StoreDependentVariableData(const FString& VarName, const FString& Value)
{
	USFDependentVariable* DependentVariable = GetDependentVarForDataStoring(VarName, Value);
	if(!DependentVariable)
	{
		return false;
	}
	FString SanitizedValue = Value;
	SanitizeValueForCSV(SanitizedValue);
	DependentVariable->Value = SanitizedValue;
	return true;
}

bool USFCondition::StoreMultipleTrialDependentVariableData(const FString& VarName, const TArray<FString>& Values)
{
	//construct a string representation of the data for error logging
	FString Data = "";
	for(const FString& Value : Values)
	{
		Data += (Data.IsEmpty() ? "{" : ",") + Value;
	}
	Data += "}";

	USFDependentVariable* DependentVariable = GetDependentVarForDataStoring(VarName, Data);
	if(!DependentVariable)
	{
		return false;
	}
	USFMultipleTrialDependentVariable* TrialDependentVar = Cast<USFMultipleTrialDependentVariable>(DependentVariable);
	if (DependentVariable && !TrialDependentVar)
	{
		FSFLoggingUtils::Log(
			"Cannot log data '" + Data + "' for multiple trial dependent variable '" + VarName +
			"' since it is not a USFMultipleTrialDependentVariable but just USFDependentVariable!", true);
		return false;
	}
	if (TrialDependentVar->SubVariableNames.Num() != Values.Num())
	{
		FSFLoggingUtils::Log(
			"Cannot log data '" + Data + "' for multiple trial dependent variable '" + VarName +
			"' since it has a wrong amount of entries, " + FString::FromInt(TrialDependentVar->SubVariableNames.Num()) +
			" were expected!", true);
		return false;
	}

	TArray<FString> SanitizedValues = Values;
	for(FString& Value : SanitizedValues)
	{
		SanitizeValueForCSV(Value);
	}
	USFGameInstance::Get()->GetParticipant()->StoreTrialInTrialDVLongTable(TrialDependentVar, SanitizedValues);

	std::vector<FString> ValuesVector;
	for(const FString& Value : SanitizedValues)
	{
		ValuesVector.push_back(Value);
	}
	TrialDependentVar->Values.push_back(ValuesVector);
	
	return true;
}

USFDependentVariable* USFCondition::GetDependentVarForDataStoring(const FString& VarName, const FString& Data)
{
	if (!WasStarted())
	{
		FSFLoggingUtils::Log(
			"Cannot log data '" + Data + "' for dependent variable '" + VarName +
			"' since condition was not started yet, probably still fading!", true);
		return nullptr;
	}

	if (IsFinished())
	{
		FSFLoggingUtils::Log(
			"Cannot log data '" + Data + "' for dependent variable '" + VarName +
			"' since condition was has finished, probably already fading!", true);
		return nullptr;
	}

	for (auto& Var : DependentVariables)
	{
		if (Var->Name == VarName)
		{
			return Var;
		}
	}

	FSFLoggingUtils::Log(
		"Cannot log data '" + Data + "' for dependent variable '" + VarName +
		"' since it does not exist for this condition!", true);
	return nullptr;
}

bool USFCondition::SanitizeValueForCSV(FString& Value)
{
	if(Value.Contains(","))
	{
		FSFLoggingUtils::Log("Cannot log data containing a ',' into a csv file, replacing ',' with [Komma] in \"" + Value + "\"", false);
		Value.ReplaceInline(TEXT(","), TEXT("[Komma]"));
		return false;
	}
	return true;
}

float USFCondition::GetTimeTaken() const
{
	return TimeTaken;
}

bool USFCondition::IsFinished() const
{
	return bConditionFinished;
}

bool USFCondition::HasRequiredVariables() const
{
	for (auto Var : DependentVariables)
	{
		if (Var->bRequired)
		{
			return true;
		}
	}
	return false;
}

bool USFCondition::WasStarted() const
{
	return bStarted;
}

bool USFCondition::RecoverStudyResults(TArray<FString>& Header, TArray<FString>& Entries)
{
	check(Header.Num()==Entries.Num())

	for (auto FactorLevel : FactorLevels)
	{
		if (!Header.Contains(FactorLevel.Key) || Entries[Header.Find(FactorLevel.Key)] != FactorLevel.Value)
		{
			return false;
		}
	}

	//so this is the right condition
	for (auto& DepVar : DependentVariables)
	{
		if(Cast<USFMultipleTrialDependentVariable>(DepVar))
		{
			//this is done in USFMultipleTrialDependetVariable::RecoverStudyResults()
			continue;
		}
		DepVar->Value = Entries[Header.Find(DepVar->Name)];
	}
	TimeTaken = FCString::Atof(*Entries[Entries.Num()-1]);
	bConditionFinished = true;

	return true;
}

FString USFCondition::GetPrettyName()
{
	FString ConditionString = "(";
	ConditionString += "Phase: " + PhaseName;
	for (auto FactorLevel : FactorLevels)
	{
		ConditionString += "; " + FactorLevel.Key + ": " + FactorLevel.Value;
	}
	ConditionString += ")";
	return ConditionString;
}

void USFCondition::Begin()
{
	StartTime = FPlatformTime::Seconds();

	for (auto& Var : DependentVariables)
	{
		if(auto MultiTrialVar = Cast<USFMultipleTrialDependentVariable>(Var))
		{
			MultiTrialVar->Values.clear();
		}
		Var->Value = "";
	}

	bStarted = true;
	bConditionFinished = false;
}

TArray<FString> USFCondition::End()
{
	const double EndTime = FPlatformTime::Seconds();

	TArray<FString> UnfinishedVars;
	for (auto Var : DependentVariables)
	{
		if(auto MultiTrial = Cast<USFMultipleTrialDependentVariable>(Var))
		{
			if(MultiTrial->bRequired && MultiTrial->Values.size()==0)
			{
				UnfinishedVars.Add(Var->Name);
			}
		}
		else if (Var->bRequired && Var->Value == "")
		{
			UnfinishedVars.Add(Var->Name);
		}
	}
	if(UnfinishedVars.Num() != 0)
	{
		return UnfinishedVars;
	}

	TimeTaken = EndTime - StartTime;
	bConditionFinished = true;

	USFLoggingBPLibrary::LogComment("EndCondition");

	return {};
}


