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


bool USFGazeTracker::Tick(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Tick ds: %f"), DeltaTime)

	TimeSinceLastEyeDataGather += DeltaTime;
	if (TimeSinceLastEyeDataGather < EyeDataGatheringDelay)
	{
		return true;
	}

	TimeSinceLastEyeDataGather = 0.0f;

	if(bEyeTrackingStarted)
	{
#ifdef WITH_SRANIPAL
		ViveSR::anipal::Eye::GetEyeData_v2(&SranipalEyeData);
		bDataLogged = false;
#endif
	}
	return true;
}


void USFGazeTracker::Init(EGazeTrackerMode Mode, bool IgnoreNonGazeTargetActors, float DataGatheringsPerSecond)
{
	bIgnoreNonGazeTargetActors = IgnoreNonGazeTargetActors;
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

		TimeSinceLastEyeDataGather = 0.0f;
		if (DataGatheringsPerSecond > 0.0f) {
			EyeDataGatheringDelay = 1.0f / DataGatheringsPerSecond;
		}
		else
		{
			EyeDataGatheringDelay = 0.0f;
		}
	}

	TickDelegate = FTickerDelegate::CreateUObject(this, &USFGazeTracker::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);

}

FGazeRay USFGazeTracker::GetLocalGazeDirection()
{
	if(!bEyeTrackingStarted)
	{
		// if no eye tracker is used we always "look ahead"
		return FGazeRay();
	}
#ifdef WITH_SRANIPAL
	return GetSranipalGazeRayFromData();
	//USRanipalEye_FunctionLibrary::GetGazeRay(GazeIndex::COMBINE, Origin, Direction);
#endif
	return FGazeRay();
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
	const float Distance = 10000.0f;

	FGazeRay GazeRay = GetWorldGazeDirection();

	const FVector RayCastOrigin = GazeRay.Origin;
	const FVector RayCastEnd = (GazeRay.Direction * Distance) + RayCastOrigin;

	//FSFUtils::Log("Cast Ray from "+RayCastOrigin.ToString()+" to "+RayCastEnd.ToString());

	FHitResult HitResult;
	UWorld* World = USFGameInstance::Get()->GetWorld();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(World->GetFirstPlayerController()->AcknowledgedPawn);
	QueryParams.AddIgnoredActor(USFGameInstance::Get()->GetHUD()->GetHUDHelper());
	ActorsToIgnore.Remove(nullptr);//remove all invalid actors first
	QueryParams.AddIgnoredActors(ActorsToIgnore);

	if (bDebugRenderRayTraces)
	{
		//this line trace is more comfortable for debug drawing, however has problems with channel tracing, so we use LineTraceSingleByChannel() above
		FHitResult TmpHitRes;
		UKismetSystemLibrary::LineTraceSingle(World, RayCastOrigin, RayCastEnd, ETraceTypeQuery::TraceTypeQuery1, false, {}, EDrawDebugTrace::ForDuration, TmpHitRes, true);
	}

	while (true) {
		World->LineTraceSingleByChannel(HitResult, RayCastOrigin, RayCastEnd, ECC_Visibility, QueryParams);

		if (HitResult.bBlockingHit && HitResult.GetActor())
		{
			//we hit something check whether the hit component is one of our SFGazeTarget components
			USFGazeTarget* GazeTarget = Cast<USFGazeTarget>(HitResult.GetComponent());
			if (GazeTarget)
			{
				return GazeTarget->GetTargetName();
			}

			//otherwise the whole actor might be a target:
			if (HitResult.GetActor())
			{
				USFGazeTargetActor* GazeTargetActor = Cast<USFGazeTargetActor>(HitResult.GetActor()->GetComponentByClass(USFGazeTargetActor::StaticClass()));
				if (GazeTargetActor)
				{
					return GazeTargetActor->GetTargetName();
				}
			}
			if(!bIgnoreNonGazeTargetActors)
			{
				//if we do not want to ignore non-gazetarget actors we are done here after running this loop once
				return "";
			}
			ActorsToIgnore.Add(HitResult.GetActor());
			QueryParams.AddIgnoredActor(HitResult.GetActor());
		}
		else
		{
			break;
		}
	}



	return "";
}

bool USFGazeTracker::LaunchCalibration()
{
#ifdef WITH_SRANIPAL
	int Result = ViveSR::anipal::Eye::LaunchEyeCalibration(nullptr);
	if(Result == ViveSR::Error::WORK)
	{
		return true;
	}
	USFLoggingBPLibrary::LogComment("Eye Tracking Calibration failed with error code: " + FString::FromInt(Result) + " (see ViveSR::Error for coding).");
	return false;
#endif
	USFLoggingBPLibrary::LogComment("Eye Tracking Calibration cannot be performed, SRAnipalPlugin not present.");
	return false;
}

bool USFGazeTracker::IsTrackingEyes()
{
	if (!bEyeTrackingStarted)
		return false;

#ifdef WITH_SRANIPAL
	//no_user apparently is true, when a user is there... (weird but consistent)
	return SranipalEyeData.no_user;
#endif

	return false;
}

float USFGazeTracker::GetEyesOpenness()
{
	if(!IsTrackingEyes())
	{
		return -1.0f;
	}

#ifdef WITH_SRANIPAL
	return 0.5f * (SranipalEyeData.verbose_data.left.eye_openness + SranipalEyeData.verbose_data.right.eye_openness);
#endif

	return -1.0f;
}

bool USFGazeTracker::DataAlreadyLogged()
{
	return bDataLogged;
}

void USFGazeTracker::SetDataLogged()
{
	bDataLogged = true;
}

float USFGazeTracker::GetPupilDiameter()
{
	if (!IsTrackingEyes())
	{
		return 0.0f;
	}

#ifdef WITH_SRANIPAL
	return 0.5f * (SranipalEyeData.verbose_data.left.pupil_diameter_mm + SranipalEyeData.verbose_data.right.pupil_diameter_mm);
#endif

	return 0.0f;
}

FGazeRay USFGazeTracker::GetSranipalGazeRayFromData()
{
	FGazeRay GazeRay;

	//this is copied from SRanipalEye_Core.cpp GetGazeRay() (case gazeIndex == GazeIndex::COMBINE)
	bool bValid = SranipalEyeData.verbose_data.left.GetValidity(ViveSR::anipal::Eye::SingleEyeDataValidity::SINGLE_EYE_DATA_GAZE_DIRECTION_VALIDITY)
		&& SranipalEyeData.verbose_data.right.GetValidity(ViveSR::anipal::Eye::SingleEyeDataValidity::SINGLE_EYE_DATA_GAZE_DIRECTION_VALIDITY);
	if (bValid)
	{
		FVector SranipalOrigin = (SranipalEyeData.verbose_data.left.gaze_origin_mm + SranipalEyeData.verbose_data.right.gaze_origin_mm) / 1000 / 2;
		FVector SRanipalDirection = (SranipalEyeData.verbose_data.left.gaze_direction_normalized + SranipalEyeData.verbose_data.right.gaze_direction_normalized) / 2;

		//CovertToUnrealLocation(origin)
		GazeRay.Origin.X = SranipalOrigin.Z;
		GazeRay.Origin.Y = SranipalOrigin.X * -1;
		GazeRay.Origin.Z = SranipalOrigin.Y;
		//ApplyUnrealWorldToMeterScale(origin);
		float Scale = GWorld ? GWorld->GetWorldSettings()->WorldToMeters : 100.f;
		GazeRay.Origin *= Scale;
		//CovertToUnrealLocation(direction);
		GazeRay.Direction.X = SRanipalDirection.Z;
		GazeRay.Direction.Y = SRanipalDirection.X;
		GazeRay.Direction.Z = SRanipalDirection.Y;
	}
	return GazeRay;
}
