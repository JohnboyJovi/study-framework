#pragma once

#include "CoreMinimal.h"
#include "SFDefinesPublic.h"

#include "SFStudyPhase.generated.h"

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFStudyPhase : public UObject
{
	GENERATED_BODY()

public:
	USFStudyPhase();

	UFUNCTION()
	void AddStudyFactor(FSFStudyFactor Setting);
	UFUNCTION()
	void AddMap(FString Name);


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
	TArray<int> NextCondition(); // TODO implement NextCondition()

	UFUNCTION()
	bool ApplyCondition();

	// Getter

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
	TArray<int> GetCurrentCondition();

	UFUNCTION()
	const TArray<FString>& GetMapNames() const;

	UFUNCTION()
	const TArray<FSFStudyFactor>& GetFactors() const;


private:

	UPROPERTY()
	TArray<FString> MapNames;

	UPROPERTY()
	TArray<FSFStudyFactor> Factors;


	// Repititions
	UPROPERTY()
	int NumberOfRepetitions = 1;

	UPROPERTY()
	TEnumAsByte<EPhaseRepetitionType> TypeOfRepetition = EPhaseRepetitionType::SameOrder;


	// Mixing Stuff
	UPROPERTY()
	TEnumAsByte<EMixingSetupOrder> TypeOfMixing = EMixingSetupOrder::RandomSetupOrder;

	//per condition the order array holds first the index of the map and then levels of each factor
	TArray<TArray<int>> Orders;

	UPROPERTY()
	TArray<int> UpcomingCondition;

	UPROPERTY()
	TArray<int> CurrentCondition;

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
