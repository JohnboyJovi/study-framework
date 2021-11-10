#pragma once

#include "CoreMinimal.h"
#include "SFDefines.h"

#include "SFStudyFactor.h"
#include "SFDependentVariable.h"

#include "SFStudyPhase.generated.h"

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFStudyPhase : public UObject
{
	GENERATED_BODY()

public:
	USFStudyPhase();

	//
	// Setting up the Study Phase
	//
	
	UFUNCTION()
	USFStudyFactor* AddStudyFactor(FString FactorName, TArray<FString> FactorLevels);
	UFUNCTION()
	void AddMap(FString Name);
	UFUNCTION()
	void AddDependentVariable(FString Name);


	UFUNCTION()
	void AddActorForEveryLevelInThisPhaseCpp(UClass* Actor);
	UFUNCTION()
	void AddActorForEveryLevelInThisPhaseBlueprint(FSFClassOfBlueprintActor Actor);


	UFUNCTION()
	void SetRepetitions(int Num, EPhaseRepetitionType RepetitionType);
	UFUNCTION()
	void SetSettingsMixing(EMixingSetupOrder MixingType);


	

	UFUNCTION()
	bool PhaseValid(); // TODO implement PhaseValid()

	UFUNCTION()
	bool GenerateOrder(); // TODO implement GenerateOrder()

	// Prepare everything for next setup and load map.
	UFUNCTION()
	TArray<FString> NextCondition(); // TODO implement NextCondition()

	UFUNCTION()
	bool ApplyCondition();

	//
	// Getter
	//
	
	UFUNCTION()
	FString GetUpcomingLevelName() const;

	UFUNCTION()
	TArray<UClass*> GetSpawnActorsCpp() const;

	UFUNCTION()
	TArray<FSFClassOfBlueprintActor> GetSpawnActorsBlueprint() const;

	UFUNCTION()
	TArray<int> GetFactorsLevelCount();

	UFUNCTION()
	TArray<FString> GetOrderStrings(); // TODO implement

	UFUNCTION()
	TArray<FString> GetCurrentCondition();

	UFUNCTION()
	const TArray<FString>& GetMapNames() const;

	UFUNCTION()
	const TArray<USFStudyFactor*> GetFactors() const;


private:

	void CreateAllOrdersRecursively(int Index, TArray<int> OrderPart, TArray<TArray<int>>& OrdersIndices);

	UPROPERTY()
	TArray<FString> MapNames;

	UPROPERTY()
	TArray<USFStudyFactor*> Factors;

	UPROPERTY()
	TArray<USFDependentVariable*> DependentVariables;


	// Repititions
	UPROPERTY()
	int NumberOfRepetitions = 1;

	UPROPERTY()
	TEnumAsByte<EPhaseRepetitionType> TypeOfRepetition = EPhaseRepetitionType::SameOrder;


	// Mixing Stuff
	UPROPERTY()
	TEnumAsByte<EMixingSetupOrder> TypeOfMixing = EMixingSetupOrder::RandomSetupOrder;

	//per condition the order array holds first the index of the map and then levels of each factor
	TArray<TArray<FString>> Orders;

	UPROPERTY()
	TArray<FString> UpcomingCondition;

	UPROPERTY()
	TArray<FString> CurrentCondition;

	UPROPERTY()
	int CurrentCondtitionIdx = -1;

	UPROPERTY()
	FString UpcomingMapName;

	// Spawn on Level
	UPROPERTY()
	TArray<UClass*> SpawnInThisPhaseCpp;

	UPROPERTY()
	TArray<FSFClassOfBlueprintActor> SpawnInThisPhaseBlueprint;
};


