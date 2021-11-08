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
	USFStudyPhase* AddStudyPhase(FName PhaseName);

	UFUNCTION()
	bool CheckPhases();

	UFUNCTION()
	int GetNumberOfPhases();

	UFUNCTION()
	USFStudyPhase* GetPhase(int Index);

protected:
	UPROPERTY()
	TArray<USFStudyPhase*> Phases;
};
