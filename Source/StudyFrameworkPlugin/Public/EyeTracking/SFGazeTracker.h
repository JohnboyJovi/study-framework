#pragma once

#include "CoreMinimal.h"

#include "SFGazeTracker.generated.h"

USTRUCT(BlueprintType)
struct FGazeRay
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	FVector Origin;
	UPROPERTY(BlueprintReadOnly)
	FVector Direction;
};

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFGazeTracker : public UObject
{
	GENERATED_BODY()

public:
	void Init();

	//returns pair of Origin and Direction
	UFUNCTION(BlueprintCallable)
	FGazeRay GetGazeDirection();

	//returns the name of the gaze target or empty string if none found or not tracking
	UFUNCTION(BlueprintCallable)
	FString GetCurrentGazeTarget();

	UFUNCTION(BlueprintCallable)
	FString LaunchCalibration();

	UFUNCTION(BlueprintCallable)
	bool IsTracking();

private:

	bool bIsStarted = false;
};
