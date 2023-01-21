#include "GazeTracking/SFGazeTarget.h"

const FString USFGazeTarget::GetTargetName() const
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

void USFGazeTarget::OnComponentCreated()
{
	Super::OnComponentCreated();

	//we use GameTraceChannel 1 for gazing traces
	SetCollisionObjectType(ECC_Visibility);
	SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}


