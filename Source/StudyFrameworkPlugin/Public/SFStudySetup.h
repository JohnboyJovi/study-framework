#pragma once

#include "CoreMinimal.h"
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"

#include "SFStudyPhase.h"
#include "HUD/SFFadeHandler.h"
#include "HUD/SFExperimenterWindow.h"

#include "SFStudySetup.generated.h"

UCLASS(HideCategories=(Transform, Rendering, Replication, Collision, Input, Actor, LOD, Cooking))
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
	bool CheckPhases() const;

	//This methods generates NrOfRunsToGenerate study runs and puts them into StudyFrame/StudyRuns
	//it can be used to chek randomization etc. manually to see what participant sees which conditions
	UFUNCTION(CallInEditor, Category = "Study Setup Debug")
	void GenerateTestStudyRuns() const;
	UPROPERTY(EditAnywhere, Category = "Study Setup Debug")
	int NrOfRunsToGenerate = 1;

	//CAUTION: this removes results, should only be called to prepare before starting the actual study!
	UFUNCTION(CallInEditor, Category = "Study Setup Debug")
	void ClearStudyResults() const;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup|Fading"))
	FFadeConfig FadeConfig;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup|Experimenter View"))
	FExperimenterViewConfig ExperimenterViewConfig;


	// ****************************************************************** // 
	// ******* Getters ************************************************** //
	// ****************************************************************** //

	UFUNCTION()
	TArray<USFCondition*> GetAllConditionsForRun(int RunningParticipantNumber) const;
	
	UFUNCTION()
	int GetNumberOfPhases();

	UFUNCTION()
	USFStudyPhase* GetPhase(int Index);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup Json Storage"))
	FString JsonFile = "StudySetup.json";

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Study Setup Json Storage")
	void LoadFromJson();

protected:

	TSharedPtr<FJsonObject> GetAsJson() const;
	void FromJson(TSharedPtr<FJsonObject> Json);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Study Setup Json Storage")
	void SaveToJson() const;

	bool ContainsNullptrInArrays();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced, meta = (TitleProperty = "PhaseName", Category = "Study Setup"))
	TArray<USFStudyPhase*> Phases;

	
	// A UBillboardComponent to hold Icon sprite
	UPROPERTY()
	UBillboardComponent* SpriteComponent;

	// Icon sprite
	UPROPERTY()
	UTexture2D* SpriteTexture;;

};
