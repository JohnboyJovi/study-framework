#include "EyeTracking/SFEyeTracker.h"

#include "SFGameInstance.h"
#include "EyeTracking/SFGazeTarget.h"
#include "Help/SFUtils.h"

#ifdef WITH_SRANIPAL
#include "SRanipalEye_FunctionLibrary.h"
#endif


void USFEyeTracker::Init()
{
#ifdef WITH_SRANIPAL
	//FSFUtils::OpenMessageBox("SRanipal Plugin found!", false);
	USRanipalEye_FunctionLibrary::StartEyeFramework(SupportedEyeVersion::version2);
#else
	FSFUtils::OpenMessageBox("SRanipal Plugin is not present, cannot use eye tracking! Check out, e.g., StudyFramework Wiki where to get it", true);
#endif
}

FGazeRay USFEyeTracker::GetGazeDirection()
{
	FVector Origin;
	FVector Direction;
	USRanipalEye_FunctionLibrary::GetGazeRay(GazeIndex::COMBINE, Origin, Direction);
	FGazeRay GazeRay;
	GazeRay.Origin = Origin;
	GazeRay.Direction = Direction;
	return GazeRay;
}

FString USFEyeTracker::GetCurrentGazeTarget()
{
	FVector GazeOrigin;
	FVector GazeDirection;
	const float Distance = 1000.0f;
	USRanipalEye_FunctionLibrary::GetGazeRay(GazeIndex::COMBINE, GazeOrigin, GazeDirection);
	//the gaze ray is relative to the HMD
	const APlayerCameraManager* CamManager = USFGameInstance::Get()->GetWorld()->GetFirstPlayerController()->
	                                                                 PlayerCameraManager;
	const FVector CameraLocation = CamManager->GetCameraLocation();
	const FRotator CameraRotation = CamManager->GetCameraRotation();
	const FVector RayCastOrigin = CameraLocation;
	const FVector RayCastEnd = (CameraRotation.RotateVector(GazeDirection) * Distance) + RayCastOrigin;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, RayCastOrigin, RayCastEnd, EYE_TRACKING_TRACE_CHANNEL);
	if (HitResult.bBlockingHit)
	{
		//we hit something check whether it is one of our SFGazeTarget components
		USFGazeTarget* GazeTarget = Cast<USFGazeTarget>(HitResult.Component);
		if (GazeTarget)
		{
			return GazeTarget->TargetName;
		}
	}
	return "nothing";
}
