#pragma once

#include "CoreMinimal.h"
#include "SFLogCustomDataComponent.generated.h"

//this component can be used as a tracked component if additional custom data should be logged in the position log
//if more than one custom value should be written to the PositionLog[...].txt just add them to the string separated by "\t"

UCLASS(meta = (BlueprintSpawnableComponent))
class STUDYFRAMEWORKPLUGIN_API USFLogCustomDataComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	USFLogCustomDataComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomData;
};
