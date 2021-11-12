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

	// ****************************************************************** // 
	// ******* Setting up the Study ************************************* //
	// ****************************************************************** //

	UFUNCTION()
	USFStudyPhase* AddStudyPhase(FName PhaseName);

	UFUNCTION()
	void AddActorForEveryLevelInEveryPhase(TSubclassOf<AActor> Actor);
	UFUNCTION()
	void AddActorForEveryLevelInEveryPhaseBlueprint(const FString& BlueprintPath, const FString& BlueprintName);

	UFUNCTION()
	bool CheckPhases();

	// ****************************************************************** // 
	// ******* Getters ************************************************** //
	// ****************************************************************** //

	UFUNCTION()
	TArray<USFCondition*> GetAllConditionsForRun(int RunningParticipantNumber);
	
	UFUNCTION()
	int GetNumberOfPhases();

	UFUNCTION()
	USFStudyPhase* GetPhase(int Index);

	UFUNCTION()
	TArray<TSubclassOf<AActor>> GetSpawnActors() const;

	TSharedPtr<FJsonObject> GetAsJson() const;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<USFStudyPhase*> Phases;

	// Spawn in every Level of every phase
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<TSubclassOf<AActor>> SpawnInEveryPhase;
};
