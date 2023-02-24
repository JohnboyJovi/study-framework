#pragma once

#include "CoreMinimal.h"

#include "SFDependentVariable.generated.h"

UCLASS(EditInlineNew)
class STUDYFRAMEWORKPLUGIN_API USFDependentVariable : public UObject
{
	GENERATED_BODY()

public:
	USFDependentVariable();

	virtual TSharedPtr<FJsonObject> GetAsJson() const;
	static USFDependentVariable* FromJson(TSharedPtr<FJsonObject> Json, UObject* Outer);

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString Name = "undefined";

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bRequired = true; //potentially there are variables that are not required in every execution

	FString Value = "";

protected:

	virtual void FromJsonInternal(TSharedPtr<FJsonObject> Json);
};
