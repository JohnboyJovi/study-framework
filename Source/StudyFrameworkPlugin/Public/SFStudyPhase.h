#pragma once

#include "CoreMinimal.h"

#include "SFStudyFactor.h"
#include "SFMapFactor.h"
#include "SFCondition.h"
#include "SFDependentVariable.h"

#include "SFStudyPhase.generated.h"

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

	// ****************************************************************** // 
	// ******* Preparing the Study Phase ******************************** //
	// ****************************************************************** //

	UFUNCTION()
	bool PhaseValid() const;

	UFUNCTION()
	TArray<USFCondition*> GenerateConditions(int ParticipantNr, int PhaseIndex);


	// ****************************************************************** // 
	// ******* Getter for the Study Phase ******************************* //
	// ****************************************************************** //

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
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced, meta = (TitleProperty = "FactorName"))
	TArray<USFStudyFactor*> Factors;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced, meta = (TitleProperty = "Name"))
	TArray<USFDependentVariable*> DependentVariables;


private:
	void CreateAllConditionsRecursively(int Index, TArray<int> TmpOrderPart, TArray<USFStudyFactor*>& InSortedFactors,
	                                    int ParticipantID, TArray<TArray<int>>& OutOrdersIndices) const;

	bool ContainsAMapFactor() const;
	USFMapFactor* GetMapFactor() const;
	int GetMapFactorIndex() const;

	//puts any enBlock factor first (if one exists), then inOrder factors and then the rest
	TArray<USFStudyFactor*> SortFactors() const;
};
