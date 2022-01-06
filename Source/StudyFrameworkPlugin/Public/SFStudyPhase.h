#pragma once

#include "CoreMinimal.h"

#include "SFStudyFactor.h"
#include "SFMapFactor.h"
#include "SFCondition.h"
#include "SFDependentVariable.h"

#include "SFStudyPhase.generated.h"

UENUM()
enum class EPhaseRepetitionType : uint8
{
	SameOrder = 0 UMETA(DisplayName = "SameOrder: Repeat all conditions in the same order again NumberOfRepetitions times"),
	DifferentOrder = 1 UMETA(DisplayName = "DifferentOrder: Each repetition block is shuffeled, but 2nd repetitions are only done after each condition was seen once, and so on"),
	FullyRandom = 2 UMETA(DisplayName = "FullyRandom: Repeat all conditions NumberOfRepetitions times, but in an arbitrary order")
};

UCLASS(EditInlineNew, CollapseCategories)
class STUDYFRAMEWORKPLUGIN_API USFStudyPhase : public UObject
{
	GENERATED_BODY()

public:
	USFStudyPhase();

	// ****************************************************************** // 
	// ******* Setting up the Study Phase ******************************* //
	// ****************************************************************** //
	
	UFUNCTION()
	USFStudyFactor* AddStudyFactor(FString FactorName, const TArray<FString>& FactorLevels);
	UFUNCTION()
	USFMapFactor* AddMapFactor(const TArray<FString>& FactorLevels);
	UFUNCTION()
	void AddDependentVariable(FString Name);


	UFUNCTION()
	void AddActorForEveryMapInThisPhase(TSubclassOf<AActor> Actor);
	UFUNCTION()
	void AddBlueprintActorForEveryMapInThisPhase(const FString& BlueprintPath, const FString& BlueprintName);


	UFUNCTION()
	void SetRepetitions(int Num, EPhaseRepetitionType RepetitionType);

	// ****************************************************************** // 
	// ******* Preparing the Study Phase ******************************** //
	// ****************************************************************** //
	
	UFUNCTION()
	bool PhaseValid() const;

	UFUNCTION()
	TArray<USFCondition*> GenerateConditions(int ParticipantNr); 


	// ****************************************************************** // 
	// ******* Getter for the Study Phase ******************************* //
	// ****************************************************************** //

	UFUNCTION()
	TArray<TSubclassOf<AActor>> GetSpawnActors() const;

	UFUNCTION()
	const TArray<USFStudyFactor*> GetFactors() const;

	TSharedPtr<FJsonObject> GetAsJson() const;
	void FromJson(TSharedPtr<FJsonObject> Json);

	// ****************************************************************** // 
	// ******* Properties of this Phase ********************************* //
	// ****************************************************************** //

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString PhaseName;

	bool ContainsNullptrInArrays();

protected:
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced,  meta = (TitleProperty = "FactorName"))
	TArray<USFStudyFactor*> Factors;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced,  meta = (TitleProperty = "Name"))
	TArray<USFDependentVariable*> DependentVariables;

	// Repetitions
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int NumberOfRepetitions = 1;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EPhaseRepetitionType TypeOfRepetition = EPhaseRepetitionType::SameOrder;

	// Spawn in every level once loaded
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<TSubclassOf<AActor>> SpawnInEveryMapOfThisPhase;

private:
	
	void CreateAllConditionsRecursively(int Index, TArray<int> OrderPart, TArray<USFStudyFactor*>& SortedFactors, TArray<TArray<int>>& OrdersIndices) const;

	bool ContainsAMapFactor() const;
	USFMapFactor* GetMapFactor() const;
	int GetMapFactorIndex() const;
	bool SortFactors(TArray<USFStudyFactor*> &SortedFactors) const;
};


