#pragma once

#include "CoreMinimal.h"

#include "SFGazeTracker.generated.h"

UENUM()
enum class EGazeTrackerMode : uint8
{
	NotTracking,
	HeadRotationOnly,
	EyeTracking
};

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
	void Init(EGazeTrackerMode Mode);

	//returns pair of Origin and Direction, relative to the head!
	UFUNCTION(BlueprintCallable)
	FGazeRay GetGazeDirection();

	//returns the name of the gaze target or empty string if none found or not tracking
	UFUNCTION(BlueprintCallable)
	FString GetCurrentGazeTarget();

	UFUNCTION(BlueprintCallable)
	void LaunchCalibration();

	UFUNCTION(BlueprintCallable)
	bool IsTrackingEyes();

private:

	bool bEyeTrackingStarted = false;
};
