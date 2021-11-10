#pragma once

#include "CoreMinimal.h"

#include "SFDependentVariable.generated.h"

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFDependentVariable : public UObject
{
	GENERATED_BODY()

public:
	USFDependentVariable();

	UPROPERTY()
	FString Name = "undefined";

	UPROPERTY()
	FString Value="";
};
