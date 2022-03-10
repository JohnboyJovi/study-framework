#include "EyeTracking/SFGazeTarget.h"

void USFGazeTarget::OnComponentCreated()
{
	Super::OnComponentCreated();

	Collision = NewObject<USphereComponent>(this, "GazeSphereCollision");
	Collision->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

	//we use GameTraceChannel 1 for gazing traces
	Collision->SetCollisionObjectType(ECC_GameTraceChannel1);
}


