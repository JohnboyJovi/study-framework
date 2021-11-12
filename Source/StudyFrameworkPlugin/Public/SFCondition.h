#pragma once

#include "CoreMinimal.h"

#include "SFStudyFactor.h"
#include "SFDependentVariable.h"


#include "SFCondition.generated.h"

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFCondition : public UObject
{
	GENERATED_BODY()

public:
	USFCondition();

	void Generate(const FString& InPhaseName, const TArray<int>& ConditionIndices, const TArray<USFStudyFactor*>& Factors, const TArray<USFDependentVariable*>& InDependentVars);

	TSharedPtr<FJsonObject> GetAsJson() const;

	static FString CreateIdentifiableName(const FString& PhaseName, const TArray<int>& ConditionIndices);

	FString ToString() const;

	bool operator==(USFCondition &Other);

	UPROPERTY(BlueprintReadOnly)
	FString PhaseName;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString,FString> FactorLevels;

	UPROPERTY(BlueprintReadOnly)
	FString Map = "undefined";

	UPROPERTY(BlueprintReadOnly)
	TMap<USFDependentVariable*,FString> DependentVariablesValues;

	UPROPERTY()
	TArray<TSubclassOf<AActor>> SpawnInThisCondition;
};
