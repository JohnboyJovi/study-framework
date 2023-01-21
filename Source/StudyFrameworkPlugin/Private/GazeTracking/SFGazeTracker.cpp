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

FGazeRay USFGazeTracker::GetLocalGazeDirection()
{
	if(!bEyeTrackingStarted)
	{
		// if no eye tracker is used we always "look ahead"
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

FGazeRay USFGazeTracker::GetWorldGazeDirection()
{
	FGazeRay LocalGazeRay = GetLocalGazeDirection();

	UWorld* World = USFGameInstance::Get()->GetWorld();

	//the gaze ray is relative to the HMD
	const APlayerCameraManager* CamManager = World->GetFirstPlayerController()->
		PlayerCameraManager;
	const FVector CameraLocation = CamManager->GetCameraLocation();
	const FRotator CameraRotation = CamManager->GetCameraRotation();

	FGazeRay GazeRay;
	GazeRay.Origin = CameraLocation + CameraRotation.RotateVector(LocalGazeRay.Origin);
	GazeRay.Direction = CameraRotation.RotateVector(LocalGazeRay.Direction);

	return GazeRay;
}

FString USFGazeTracker::GetCurrentGazeTarget()
{
	FString GazedAtTarget = "";
	const float Distance = 1000.0f;

	FGazeRay GazeRay = GetWorldGazeDirection();


	const FVector RayCastOrigin = GazeRay.Origin;
	const FVector RayCastEnd = (GazeRay.Direction * Distance) + RayCastOrigin;

	//FSFUtils::Log("Cast Ray from "+RayCastOrigin.ToString()+" to "+RayCastEnd.ToString());

	FHitResult HitResult;
	UWorld* World = USFGameInstance::Get()->GetWorld();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(World->GetFirstPlayerController()->AcknowledgedPawn);
	QueryParams.AddIgnoredActor(USFGameInstance::Get()->GetHUD()->GetHUDHelper());

	World->LineTraceSingleByChannel(HitResult, RayCastOrigin, RayCastEnd, ECC_Visibility, QueryParams);

	if (bDebugRenderRayTraces)
	{
		//this line trace is more comfortable for debug drawing, however has problems with channel tracing, so we use LineTraceSingleByChannel() above
		FHitResult TmpHitRes;
		UKismetSystemLibrary::LineTraceSingle(World, RayCastOrigin, RayCastEnd, ETraceTypeQuery::TraceTypeQuery1, false, {}, EDrawDebugTrace::ForDuration, TmpHitRes, true);
	}


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
	ViveSR::anipal::Eye::LaunchEyeCalibration(nullptr);
#endif
}

bool USFGazeTracker::IsTrackingEyes()
{
	if (!bEyeTrackingStarted)
		return false;

#ifdef WITH_SRANIPAL
	ViveSR::anipal::Eye::EyeData_v2 Data;
	ViveSR::anipal::Eye::GetEyeData_v2(&Data);

	//no_user apparently is true, when a user is there... (weird but consistent)
	return Data.no_user;
#endif

	return true;
}
