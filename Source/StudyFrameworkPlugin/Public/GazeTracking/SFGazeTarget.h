#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"

#include "SFGazeTarget.generated.h"

#define EYE_TRACKING_TRACE_CHANNEL ECC_GameTraceChannel4

/*
 * This makes the region covered by the sphere component a gaze target
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class STUDYFRAMEWORKPLUGIN_API USFGazeTarget : public USphereComponent
{
	GENERATED_BODY()

public:

	const FString GetTargetName() const;

	virtual void OnComponentCreated() override;

	UPROPERTY(EditAnywhere, meta=(EditCondition="!bUseActorName"))
	FString TargetName = "UndefinedTarget";

	UPROPERTY(EditAnywhere)
	bool bUseActorName = false;
};
