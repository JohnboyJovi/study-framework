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
	void Init(EGazeTrackerMode Mode, bool IgnoreNonGazeTargetActors);

	//returns pair of Origin and Direction, in world coordinates
	UFUNCTION(BlueprintCallable)
	FGazeRay GetWorldGazeDirection();

	//returns the name of the gaze target or empty string if none found or not tracking
	UFUNCTION(BlueprintCallable)
	FString GetCurrentGazeTarget();

	//returns pair of Origin and Direction, relative to the head!
	UFUNCTION(BlueprintCallable)
	FGazeRay GetLocalGazeDirection();

	//return whether calibraction was successful
	UFUNCTION(BlueprintCallable)
	bool LaunchCalibration();

	//whether currently the eyes are tracked, otherwise head-forward is used (this can change during execution if the HMD is taken off etc.)
	UFUNCTION(BlueprintCallable)
	bool IsTrackingEyes();

	// a value (mean of both eyes), how open the eyes are from 0.0 (fully closed), to 1.0 (fully opened), extreme values might not be reached
	// returns -1.0 if the eyes are not tracked or not value can be estimated
	UFUNCTION(BlueprintCallable)
	float GetEyesOpenness();

	// pupil diameter in mm averaged over both eyes
	// returns 0.0 if the eyes are not tracked or not value can be estimated
	UFUNCTION(BlueprintCallable)
	float GetPupilDiameter();

	UPROPERTY(BlueprintReadWrite)
	bool bDebugRenderRayTraces = false;

private:

	bool bEyeTrackingStarted = false;

	bool bIgnoreNonGazeTargetActors = false;
	UPROPERTY()
	TArray<AActor*> ActorsToIgnore;
};
