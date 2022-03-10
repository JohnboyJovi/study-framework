#pragma once

#include "CoreMinimal.h"

#include "SFEyeTracker.generated.h"

USTRUCT(BlueprintType)
struct FGazeRay
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	FVector Origin;
	UPROPERTY(BlueprintReadOnly)
	FVector Direction;
};

UCLASS(EditInlineNew)
class STUDYFRAMEWORKPLUGIN_API USFEyeTracker : public UObject
{
	GENERATED_BODY()

public:
	void Init();

	//returns pair of Origin and Direction
	UFUNCTION(BlueprintCallable)
	FGazeRay GetGazeDirection();

	UFUNCTION(BlueprintCallable)
	FString GetCurrentGazeTarget();
};
