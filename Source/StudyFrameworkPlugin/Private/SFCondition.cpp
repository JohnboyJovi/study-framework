#include "SFCondition.h"

#include "UObject/UObjectGlobals.h"

#include "SFMapFactor.h"
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
		DependentVariablesValues.Add(DuplicateObject(Var, this), "");
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
	for (auto Var : DependentVariablesValues)
	{
		TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Var.Key->GetAsJson());
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
		USFDependentVariable* DependentVariable = NewObject<USFDependentVariable>();
		DependentVariable->FromJson(Var->AsObject());
		DependentVariablesValues.Add(DependentVariable, "");
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
	if(!WasStarted())
	{
		FSFLoggingUtils::Log(
			"Cannot log data '" + Value + "' for dependent variable '" + VarName +
			"' since condition was not started yet, probably still fading!", true);
		return false;
	}

	if (IsFinished())
	{
		FSFLoggingUtils::Log(
			"Cannot log data '" + Value + "' for dependent variable '" + VarName +
			"' since condition was has finished, probably already fading!", true);
		return false;
	}

	for (auto& Var : DependentVariablesValues)
	{
		if (Var.Key->Name == VarName)
		{
			Var.Value = Value;
			return true;
		}
	}

	FSFLoggingUtils::Log(
		"Cannot log data '" + Value + "' for dependent variable '" + VarName +
		"' since it does not exist for this condition!", true);
	return false;
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
	for (auto Var : DependentVariablesValues)
	{
		if (Var.Key->bRequired)
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
	for (auto& DepVar : DependentVariablesValues)
	{
		DepVar.Value = Entries[Header.Find(DepVar.Key->Name)];
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

	for (auto Vars : DependentVariablesValues)
	{
		Vars.Value = "";
	}

	bStarted = true;

	//TODO: anything else to setup?
}

TArray<FString> USFCondition::End()
{
	const double EndTime = FPlatformTime::Seconds();

	TArray<FString> UnfinishedVars;
	for (auto Vars : DependentVariablesValues)
	{
		if (Vars.Key->bRequired && Vars.Value == "")
		{
			UnfinishedVars.Add(Vars.Key->Name);
		}
	}
	if(UnfinishedVars.Num() != 0)
	{
		return UnfinishedVars;
	}

	TimeTaken = EndTime - StartTime;
	bConditionFinished = true;
	return {};
}
