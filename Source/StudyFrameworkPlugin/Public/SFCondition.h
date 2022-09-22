#pragma once

#include "CoreMinimal.h"

#include "SFStudyFactor.h"
#include "SFDependentVariable.h"


#include "SFCondition.generated.h"

class USFGameInstance;

UCLASS(EditInlineNew)
class STUDYFRAMEWORKPLUGIN_API USFCondition : public UObject
{
	GENERATED_BODY()

	friend USFGameInstance;

public:
	USFCondition();

	void Generate(const FString& InPhaseName, const TArray<int>& ConditionIndices, const TArray<USFStudyFactor*>& Factors, const TArray<USFDependentVariable*>& InDependentVars);

	TSharedPtr<FJsonObject> GetAsJson() const;
	void FromJson(TSharedPtr<FJsonObject> Json);

	FString ToString() const;

	bool operator==(USFCondition &Other);

	//returns false if a variable with that name does not exist
	bool StoreDependentVariableData(const FString& VarName, const FString& Value);

	float GetTimeTaken() const;
	bool IsFinished() const;
	bool HasRequiredVariables() const;
	bool WasStarted() const;

	//this is used to recover study results from the phase long table if a participant's run is continued
	//return false if this entry does not match this condition
	bool RecoverStudyResults(TArray<FString>& Header, TArray<FString>& Entries);

	// in the form (Map:...; Factor1:Level1; ...)
	FString GetPrettyName();

	UPROPERTY(BlueprintReadOnly)
	FString UniqueName;

	UPROPERTY(BlueprintReadOnly)
	FString PhaseName;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString,FString> FactorLevels;

	UPROPERTY(BlueprintReadOnly)
	FString Map = "undefined";

	UPROPERTY(BlueprintReadOnly)
	TMap<USFDependentVariable*,FString> DependentVariablesValues;

protected:

	UPROPERTY(BlueprintReadOnly)
	float TimeTaken=0.0;

	UPROPERTY(BlueprintReadOnly)
	bool bConditionFinished=false;

	void Begin();
	bool End();

	FString CreateIdentifiableName();

private:

	bool bStarted=false;
	double StartTime;
};
