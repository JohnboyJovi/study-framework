#include "EyeTracking/SFEyeTracker.h"

#include "EyeTracking/SFGazeTarget.h"
#include "Help/SFUtils.h"
#include "Kismet/GameplayStatics.h"
#include "KismetTraceUtils.h"

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
	FVector Origin;
	FVector Direction;
	float Distance = 1000.0f;
	USRanipalEye_FunctionLibrary::GetGazeRay(GazeIndex::COMBINE, Origin, Direction);
	FHitResult HitResult;
	//KismetSystemLibrary::LineTraceSingle(GetWorld(), Origin, Origin + Distance * Direction.GetSafeNormal(), EYE_TRACKING_TRACE_CHANNEL, false, {}, EDrawDebugTrace::Persistent, HitResult, true);
	GetWorld()->LineTraceSingleByChannel(HitResult, Origin, Origin + Distance * Direction.GetSafeNormal(),
		EYE_TRACKING_TRACE_CHANNEL);
	DrawDebugLineTraceSingle(World, Start, End, DrawDebugType, bHit, OutHit, TraceColor, TraceHitColor, DrawTime);
	if (HitResult.bBlockingHit)
	{
		//we hit something check whether it is one of our SFGazeTarget components
		USFGazeTarget* GazeTarget = Cast<USFGazeTarget>(HitResult.Component);
		if (GazeTarget)
		{
			return GazeTarget->TargetName;
		}
		else
		{
			FSFUtils::Log(
				"[USFEyeTracker::GetCurrentGazeTarget] Gazing Test hit something in channel ECC_GameTraceChannel1, that is not a USFGazeTarget. Maybe double use of channel?",
				true);
		}
	}
	return "nothing";
}
