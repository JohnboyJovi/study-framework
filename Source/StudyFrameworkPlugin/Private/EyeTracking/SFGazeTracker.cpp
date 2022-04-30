#include "GazeTracking/SFGazeTracker.h"

#include "SFGameInstance.h"
#include "GazeTracking/SFGazeTarget.h"
#include "Help/SFUtils.h"

#ifdef WITH_SRANIPAL
#include "SRanipalEye_FunctionLibrary.h"
#include "Eye/SRanipal_API_Eye.h"
#include "SRanipal_API.h"
#endif


void USFGazeTracker::Init()
{
	bIsStarted = false;
#ifdef WITH_SRANIPAL
	//FSFUtils::OpenMessageBox("SRanipal Plugin found!", false);
	if(ViveSR::anipal::Eye::IsViveProEye())
	{
		USRanipalEye_FunctionLibrary::StartEyeFramework(SupportedEyeVersion::version2);
		bIsStarted = true;
	}
	FSFUtils::OpenMessageBox("[USFGazeTracker::Init] USFGazeTracker currently only works with Vive Pro Eye", true);
#else
	FSFUtils::OpenMessageBox("SRanipal Plugin is not present, cannot use eye tracking! Check out, e.g., StudyFramework Wiki where to get it", true);
#endif
}

FGazeRay USFGazeTracker::GetGazeDirection()
{
	if(!IsTracking())
	{
		FGazeRay GazeRay;
		GazeRay.Origin = FVector::ZeroVector;
		GazeRay.Direction = FVector::ForwardVector;;
		return GazeRay;
	}
	FVector Origin;
	FVector Direction;
	USRanipalEye_FunctionLibrary::GetGazeRay(GazeIndex::COMBINE, Origin, Direction);
	FGazeRay GazeRay;
	GazeRay.Origin = Origin;
	GazeRay.Direction = Direction;
	return GazeRay;
}

FString USFGazeTracker::GetCurrentGazeTarget()
{
	if(!IsTracking())
	{
		return "";
	}

	FVector GazeOrigin;
	FVector GazeDirection;
	const float Distance = 1000.0f;
	USRanipalEye_FunctionLibrary::GetGazeRay(GazeIndex::COMBINE, GazeOrigin, GazeDirection);

	//use if no I tracker is present so that we always "look ahead"
	//GazeDirection = FVector(1,0,0);

	UWorld* World = USFGameInstance::Get()->GetWorld();

	//the gaze ray is relative to the HMD
	const APlayerCameraManager* CamManager = World->GetFirstPlayerController()->
	                                                                 PlayerCameraManager;
	const FVector CameraLocation = CamManager->GetCameraLocation();
	const FRotator CameraRotation = CamManager->GetCameraRotation();
	const FVector RayCastOrigin = CameraLocation;
	const FVector RayCastEnd = (CameraRotation.RotateVector(GazeDirection) * Distance) + RayCastOrigin;

	//FSFUtils::Log("Cast Ray from "+RayCastOrigin.ToString()+" to "+RayCastEnd.ToString());

	FHitResult HitResult;
	World->LineTraceSingleByChannel(HitResult, RayCastOrigin, RayCastEnd, EYE_TRACKING_TRACE_CHANNEL);


	if (HitResult.bBlockingHit)
	{
		//we hit something check whether it is one of our SFGazeTarget components
		USFGazeTarget* GazeTarget = Cast<USFGazeTarget>(HitResult.Component);
		if (GazeTarget)
		{
			return GazeTarget->TargetName;
		}
	}
	return "";
}

FString USFGazeTracker::LaunchCalibration()
{
	//TODO: not tested yet!
	ViveSR::anipal::Eye::LaunchEyeCalibration(nullptr);
}

bool USFGazeTracker::IsTracking()
{
	//TODO: maybe use
	//ViveSR::anipal::AnipalStatus Status;
	//int Error = ViveSR::anipal::GetStatus(ViveSR::anipal::Eye::ANIPAL_TYPE_EYE_V2, &Status);
	return bIsStarted;
}
