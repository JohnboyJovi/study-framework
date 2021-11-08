#pragma once

#include "CoreMinimal.h"

#include "SFStudyFactor.generated.h"

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFStudyFactor : public UObject
{
	GENERATED_BODY()

public:
	USFStudyFactor();

	UPROPERTY()
	FString FactorName = "undefined";

	UPROPERTY()
	TArray<FString> Levels;
};
