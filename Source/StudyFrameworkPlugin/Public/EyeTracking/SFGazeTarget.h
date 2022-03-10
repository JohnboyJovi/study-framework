#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"

#include "SFGazeTarget.generated.h"

/*
 * This is a special Study Factor representing a maps/levels which are faded to
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STUDYFRAMEWORKPLUGIN_API USFGazeTarget : public USceneComponent
{
	GENERATED_BODY()

public:

	virtual void OnComponentCreated() override;

	UPROPERTY(EditAnywhere)
	FString TargetName = "UndefinedTarget";

private:
	UPROPERTY()
	USphereComponent* Collision;
};
