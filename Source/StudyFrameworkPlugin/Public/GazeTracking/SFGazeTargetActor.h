#pragma once

#include "CoreMinimal.h"

#include "SFGazeTargetActor.generated.h"

/*
 * This marks the actor it is attached to as gaze target
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class STUDYFRAMEWORKPLUGIN_API USFGazeTargetActor : public UActorComponent
{
	GENERATED_BODY()

public:

	const FString GetTargetName() const;

	UPROPERTY(EditAnywhere, meta=(EditCondition="!bUseActorName"))
	FString TargetName = "UndefinedTarget";

	UPROPERTY(EditAnywhere)
	bool bUseActorName = false;
};
