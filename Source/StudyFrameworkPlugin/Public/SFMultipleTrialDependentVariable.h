#pragma once

#include <vector>
#include "CoreMinimal.h"

#include "SFDependentVariable.h"

#include "SFMultipleTrialDependentVariable.generated.h"

class USFCondition;
// this dependent variable subtype should be used if in one condition not only one value for this dependent variable should be collected
// but multiple values of multiple trials should be recorded.
UCLASS(EditInlineNew)
class STUDYFRAMEWORKPLUGIN_API USFMultipleTrialDependentVariable : public USFDependentVariable
{
	GENERATED_BODY()

public:
	USFMultipleTrialDependentVariable();

	virtual TSharedPtr<FJsonObject> GetAsJson() const override;

	void RecoverStudyResults(USFCondition* Condition, FString ParticipantID);

	//these are all the values that should be recorded for one trial
	UPROPERTY(EditAnywhere)
	TArray<FString> SubVariableNames;

	//nested TArrays are not supported so I use std::vector here instead since it is used in C++ only anyways
	std::vector<std::vector<FString>> Values;

protected:
	virtual void FromJsonInternal(TSharedPtr<FJsonObject> Json) override;
	void RecoverStudyResultsOfLine(const TArray<FString>& Header, const TArray<FString>& Entries, USFCondition* Condition);
};
