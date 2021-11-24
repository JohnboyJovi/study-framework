#include "SFCondition.h"

#include "SFMapFactor.h"
#include "Kismet/KismetSystemLibrary.h"

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
		DependentVariablesValues.Add(Var, "");
	}
}

TSharedPtr<FJsonObject> USFCondition::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

	//TODO: implement!

	return Json;
}

FString USFCondition::CreateIdentifiableName(const FString& PhaseName, const TArray<int>& ConditionIndices)
{
	FString Name = PhaseName;
	for (int Index : ConditionIndices)
	{
		Name = Name + "_" + FString::FromInt(Index);
	}
	return Name;
}

FString USFCondition::ToString() const
{
	FString Out = PhaseName + "_" + Map;
	for (auto Level : FactorLevels)
	{
		Out = Out + "_" + Level.Value;
	}
	return Out;
}

bool USFCondition::operator==(USFCondition& Other)
{
	//this should work, since we use CreateIdentifiableName() for the Name
	return GetName() == Other.GetName();
}

void USFCondition::Begin()
{
	StartTime = UKismetSystemLibrary::GetGameTimeInSeconds(GetWorld());

	for(auto Vars : DependentVariablesValues)
	{
		Vars.Value="";
	}

	//TODO: anything else to setup?
}

bool USFCondition::End()
{
	const float EndTime = UKismetSystemLibrary::GetGameTimeInSeconds(GetWorld());

	for(auto Vars : DependentVariablesValues)
	{
		if(Vars.Key->bRequired && Vars.Value=="")
		{
			return false;
		}
	}

	TimeTaken = EndTime-StartTime;
	bConditionFinished=true;
	return true;
}
