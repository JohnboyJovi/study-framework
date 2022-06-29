#include "GazeTracking/SFGazeTargetActor.h"

#include "GazeTracking/SFGazeTarget.h"

const FString USFGazeTargetActor::GetTargetName() const
{
	if(bUseActorName)
	{
		return GetOwner()->GetName();
	}
	else
	{
		return TargetName;
	}
}



