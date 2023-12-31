#pragma once

#include "CoreMinimal.h"

#include "SFStudyPhase.h"
#include "SFIndependentVariable.h"
#include "HUD/SFFadeHandler.h"
#include "HUD/SFExperimenterWindow.h"
#include "Logging/SFLogObject.h"
#include "GazeTracking/SFGazeTracker.h"
// includes below marked as "possibly unused", but they are vital for build to succeed
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"

#include "SFStudySetup.generated.h"


UCLASS(HideCategories=(Transform, Rendering, Replication, Collision, Input, Actor, LOD, Cooking))
class STUDYFRAMEWORKPLUGIN_API ASFStudySetup : public AActor
{
	GENERATED_BODY()

public:
	ASFStudySetup();

	virtual void PostActorCreated() override;

	virtual void BeginPlay() override;

	virtual void PostLoad() override;
	virtual void PreSave(const ITargetPlatform* TargetPlatform) override;
	//void RegisterActorTickFunctions(bool bRegister) override;

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
	//it can be used to check randomization etc. manually to see what participant sees which conditions
	UFUNCTION(CallInEditor, Category = "Study Setup Debug")
	void GenerateTestStudyRuns() const;
	UPROPERTY(EditAnywhere, Category = "Study Setup Debug")
	int NrOfRunsToGenerate = 1;
	UPROPERTY(EditAnywhere, Category = "Study Setup Debug")
	TArray<FString> ExcludeFactorsFromGeneratedRunsTable;

	//CAUTION: this removes results, should only be called to prepare before starting the actual study!
	UFUNCTION(CallInEditor, Category = "Study Setup Debug")
	void ClearStudyResults() const;

	//This method can be overriden if you want to manually change the order of the conditions after they were created from the phases (with their factors)
	// Conditions:								holds those created conditions in the order defined by phases/factors
	// ParticipantSequenceNumber:	is the sequence number of the participant starting at 0 an then up to the number of participants
	//														it can be useful if you want to implement some counterbalancing yourself
	// return:										the updated order of the condition. CAUTION: deleting or creating new ones gives undefined behavior!
	virtual TArray<USFCondition*> ConditionSortingCallback(const TArray<USFCondition*>& Conditions, int ParticipantSequenceNumber) const;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup|Fading"))
	FFadeConfig FadeConfig;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup|Experimenter View"))
	FExperimenterViewConfig ExperimenterViewConfig;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup|Gaze Tracking"))
	EGazeTrackerMode UseGazeTracker = EGazeTrackerMode::NotTracking;

	// whether to ignore actors during line of sight trace which are not gaze targets
	// this can be a very helpful setting if you are using, e.g. a indoor scene and have collision
	// volumes of many unrelated actors invisibly blocking the gaze trace
	// CAREFUL: this might also allow gaze tracing through object, which actually should block the view
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup|Gaze Tracking"))
	bool bIgnoreNonGazeTargetActors = false;

	//frame rate for eye tracking, 30 means read eye data 30 per second (0 means use unreal framerate)
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup|Gaze Tracking"))
	float EyeTrackingFrameRate = 0.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced, meta = (TitleProperty = "Name", Category = "Study Setup"))
	TArray<USFIndependentVariable*> IndependentVariables;

	//give names of phases wich should be randomized in their order between participants
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup"))
	TArray<FString> PhasesToOrderRandomize;

	//whether all maps should be loaded on starting (this increases faded-out waiting during execution)
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup"))
	bool bPreloadAllMapsOnStart = true;

	//whether to use custom participants which are asked for a startup (alternative is to simply use sequence number
	//as participant ID starting at 0 and counting up for each participant.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup"))
	bool bUseCustomParticipantIDs = false;

	//this will allow studies to do no fading -in and -out when having two consecutive conditions on the same map
	//instead OnFadedIn() is directly called. This also means that BeginPlay cannot be used to set up everything according to factor levels,
	//since the map is not started/loaded anew. Developers need to take care themselves to clean up everything during OnFadedIn delegate calls
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Study Setup"))
	bool bNoFadingOnSameMap = false;

	// ****************************************************************** // 
	// ******* Getters ************************************************** //
	// ****************************************************************** //

	UFUNCTION()
	TArray<USFCondition*> GetAllConditionsForRun(int ParticipantSequenceNumber) const;
	
	UFUNCTION()
	int GetNumberOfPhases();

	UFUNCTION()
	USFStudyPhase* GetPhase(int Index);

	UPROPERTY(BlueprintReadOnly,VisibleAnywhere, meta = (Category = "Study Setup Json Storage"))
	FString JsonFile;

	UFUNCTION(BlueprintCallable)
	void LoadFromJson();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Study Setup Json Storage")
	void LoadSetupFile();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Study Setup Json Storage")
	void SaveSetupFile();

protected:

	TSharedPtr<FJsonObject> GetAsJson() const;
	void FromJson(TSharedPtr<FJsonObject> Json);

	UFUNCTION(BlueprintCallable)
	void SaveToJson() const;

	bool ContainsNullptrInArrays();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced, meta = (TitleProperty = "PhaseName", Category = "Study Setup"))
	TArray<USFStudyPhase*> Phases;

	
	// A UBillboardComponent to hold Icon sprite
	UPROPERTY()
	UBillboardComponent* SpriteComponent;

	// Icon sprite
	UPROPERTY()
	UTexture2D* SpriteTexture;
};
