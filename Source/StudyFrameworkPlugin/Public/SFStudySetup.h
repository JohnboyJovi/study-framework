#pragma once

#include "CoreMinimal.h"

#include "SFStudyPhase.h"

#include "SFStudySetup.generated.h"

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFStudySetup : public UObject
{
	GENERATED_BODY()

public:
	USFStudySetup();

	UFUNCTION()
	USFStudyPhase* AddStudyPhase(FString PhaseName);

	UFUNCTION()
	bool CheckPhases();

protected:
	UPROPERTY()
	TArray<USFStudyPhase*> Phases;
};
