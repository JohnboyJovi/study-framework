#include "SFCondition.h"

#include "UObject/UObjectGlobals.h"

#include "SFMapFactor.h"

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
		if (Factor->IsA(USFMapFactor::StaticClass()))
		{
			Map = Factor->Levels[ConditionIndices[i]];
			continue;
		}
		FactorLevels.Add(Factor->FactorName, Factor->Levels[ConditionIndices[i]]);
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
	FString Out = PhaseName + "_" + FPaths::GetBaseFilename(Map);
	for (auto Level : FactorLevels)
	{
		Out = Out + "_" + Level.Value;
	}
	return Out;
}

bool USFCondition::operator==(USFCondition& Other)
{
	//this should work, since we use CreateIdentifiableName() for the Name
	return UniqueName == Other.UniqueName;
}

bool USFCondition::StoreDependetVariableData(const FString& VarName, const FString& Value)
{
	for (auto& Var : DependentVariablesValues)
	{
		if (Var.Key->Name == VarName)
		{
			Var.Value = Value;
			return true;
		}
	}
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

FString USFCondition::GetPrettyName()
{
	FString ConditionString = "(";
	ConditionString += "Map: " + FPaths::GetBaseFilename(Map);
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

	//TODO: anything else to setup?
}

bool USFCondition::End()
{
	const double EndTime = FPlatformTime::Seconds();

	for (auto Vars : DependentVariablesValues)
	{
		if (Vars.Key->bRequired && Vars.Value == "")
		{
			return false;
		}
	}

	TimeTaken = EndTime - StartTime;
	bConditionFinished = true;
	return true;
}
