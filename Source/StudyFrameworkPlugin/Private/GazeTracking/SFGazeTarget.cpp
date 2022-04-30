#include "EyeTracking/SFGazeTarget.h"

void USFGazeTarget::OnComponentCreated()
{
	Super::OnComponentCreated();

	//we use GameTraceChannel 1 for gazing traces
	SetCollisionObjectType(EYE_TRACKING_TRACE_CHANNEL);
}


