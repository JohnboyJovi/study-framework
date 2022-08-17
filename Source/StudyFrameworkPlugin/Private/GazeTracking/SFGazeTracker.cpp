#include "GazeTracking/SFGazeTracker.h"
#include "Logging/SFLoggingBPLibrary.h"
#include "SFGameInstance.h"
#include "GazeTracking/SFGazeTarget.h"
#include "GazeTracking/SFGazeTargetActor.h"
#include "Help/SFUtils.h"

#ifdef WITH_SRANIPAL
#include "SRanipalEye_FunctionLibrary.h"
#include "Eye/SRanipal_API_Eye.h"
#include "SRanipal_API.h"
#endif


void USFGazeTracker::Init(EGazeTrackerMode Mode)
{
	bEyeTrackingStarted = false;
	//FSFUtils::OpenMessageBox("SRanipal Plugin found!", false);
	if(Mode == EGazeTrackerMode::EyeTracking)
	{
#ifdef WITH_SRANIPAL
		if(ViveSR::anipal::Eye::IsViveProEye())
		{
			USRanipalEye_FunctionLibrary::StartEyeFramework(SupportedEyeVersion::version2);
			bEyeTrackingStarted = true;
		}
		else
		{
			USFLoggingBPLibrary::LogComment("No Vive Pro Eye present, use head rotation only for gaze tracking.", true);
		}
#else
		FSFUtils::OpenMessageBox("SRanipal Plugin is not present, cannot use eye tracking! Check out, e.g., StudyFramework Wiki where to get it", true);
#endif
	}
}

FGazeRay USFGazeTracker::GetGazeDirection()
{
	if(!IsTrackingEyes())
	{
		FGazeRay GazeRay;
		GazeRay.Origin = FVector::ZeroVector;
		GazeRay.Direction = FVector::ForwardVector;;
		return GazeRay;
	}
	FVector Origin;
	FVector Direction;
#ifdef WITH_SRANIPAL
	USRanipalEye_FunctionLibrary::GetGazeRay(GazeIndex::COMBINE, Origin, Direction);
#endif
	FGazeRay GazeRay;
	GazeRay.Origin = Origin;
	GazeRay.Direction = Direction;
	return GazeRay;
}

FString USFGazeTracker::GetCurrentGazeTarget()
{
	FString GazedAtTarget = "";
	const float Distance = 1000.0f;

	FGazeRay GazeRay = GetGazeDirection();
	// if no eye tracker is used we always "look ahead"
	// GazeDirection = FVector(1,0,0);

	UWorld* World = USFGameInstance::Get()->GetWorld();

	//the gaze ray is relative to the HMD
	const APlayerCameraManager* CamManager = World->GetFirstPlayerController()->
	                                                                 PlayerCameraManager;
	const FVector CameraLocation = CamManager->GetCameraLocation();
	const FRotator CameraRotation = CamManager->GetCameraRotation();
	const FVector RayCastOrigin = CameraLocation + CameraRotation.RotateVector(GazeRay.Origin);
	const FVector RayCastEnd = (CameraRotation.RotateVector(GazeRay.Direction) * Distance) + RayCastOrigin;

	//FSFUtils::Log("Cast Ray from "+RayCastOrigin.ToString()+" to "+RayCastEnd.ToString());

	FHitResult HitResult;
	World->LineTraceSingleByChannel(HitResult, RayCastOrigin, RayCastEnd, EYE_TRACKING_TRACE_CHANNEL);


	if (HitResult.bBlockingHit)
	{
		//we hit something check whether the hit component is one of our SFGazeTarget components
		USFGazeTarget* GazeTarget = Cast<USFGazeTarget>(HitResult.GetComponent());
		if (GazeTarget)
		{
			return GazeTarget->GetTargetName();
		}

		//otherwise the whole actor might be a target:
		if(HitResult.GetActor())
		{
			USFGazeTargetActor* GazeTargetActor = Cast<USFGazeTargetActor>(HitResult.GetActor()->GetComponentByClass(USFGazeTargetActor::StaticClass()));
			if (GazeTargetActor)
			{
				return GazeTargetActor->GetTargetName();
			}
		}
	}
	return "";
}

void USFGazeTracker::LaunchCalibration()
{
#ifdef WITH_SRANIPAL
	//TODO: not tested yet!
	ViveSR::anipal::Eye::LaunchEyeCalibration(nullptr);
#endif
}

bool USFGazeTracker::IsTrackingEyes()
{
	//TODO: maybe use
	//#ifdef WITH_SRANIPAL
	//ViveSR::anipal::AnipalStatus Status;
	//int Error = ViveSR::anipal::GetStatus(ViveSR::anipal::Eye::ANIPAL_TYPE_EYE_V2, &Status);
	//#endif
	return bEyeTrackingStarted;
}
