#pragma once

#include "CoreMinimal.h"

#include "SFStudyPhase.h"
#include "HUD/SFFadeHandler.h"

#include "SFStudySetup.generated.h"

UCLASS()
class STUDYFRAMEWORKPLUGIN_API ASFStudySetup : public AActor
{
	GENERATED_BODY()

public:
	ASFStudySetup();

	virtual void BeginPlay() override;

	virtual void PostLoad() override;
	virtual void PreSave(const ITargetPlatform* TargetPlatform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//the normal one seems sufficient
	//virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	// ****************************************************************** // 
	// ******* Setting up the Study ************************************* //
	// ****************************************************************** //

	UFUNCTION()
	USFStudyPhase* AddStudyPhase(FString PhaseName);

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

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup|Fading"))
	FFadeConfig FadeConfig;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup"))
	FString JsonFile = "StudySetup.json";

	void LoadFromJson();

protected:

	TSharedPtr<FJsonObject> GetAsJson() const;
	void FromJson(TSharedPtr<FJsonObject> Json);
	
	void SaveToJson() const;

	bool ContainsNullptrInArrays();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced, meta = (TitleProperty = "PhaseName", Category = "Study Setup"))
	TArray<USFStudyPhase*> Phases;

	// Spawn in every Level of every phase
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup"))
	TArray<TSubclassOf<AActor>> SpawnInEveryPhase;

};
