/*
#pragma once

#include "CoreMinimal.h"
#include "SFDefinesPublic.h"

#include "SharedPointer.h"


#include "SFStudyPhase.generated.h"




UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFStudyPhase : public UObject
{
    GENERATED_BODY()


*/

#pragma once

#include "CoreMinimal.h"
#include "SFDefinesPublic.h"
#include "SharedPointer.h"

#include "SFStudyPhase.generated.h"

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFStudyPhase : public UObject
{
    GENERATED_BODY()

public:
    USFStudyPhase();   

    UFUNCTION()
        void AddStudySetting(FSFStudySetting Setting);
    UFUNCTION()
        void AddLevel(FString Name);


    UFUNCTION()
        void AddActorForEveryLevelInThisPhaseCpp(UClass* Actor);
    UFUNCTION()
        void AddActorForEveryLevelInThisPhaseBlueprint(FSFClassOfBlueprintActor Actor);


    UFUNCTION()
        void SetRepetitions(int Num, EPhaseRepetitionType RepetitionType);
    UFUNCTION()
        void SetSettingsMixing(EMixingSetupOrder MixingType);


    UFUNCTION()
        bool PhaseValid();                          // TODO implement PhaseValid()

    UFUNCTION()
        bool GenerateOrder();                       // TODO implement GenerateOrder()

    // Prepare everything for next setup and load map.
    UFUNCTION()
        TArray<int> NextSetup();                           // TODO implement NextSetup()

    UFUNCTION()
        bool ApplySettings();

    // Getter

    UFUNCTION()
        FString GetUpcomingLevelName() const;

    UFUNCTION()
        TArray<UClass*> GetSpawnActorsCpp() const;

    UFUNCTION()
        TArray<FSFClassOfBlueprintActor> GetSpawnActorsBlueprint() const;

    UFUNCTION()
        TArray<int> GetSetupNumArray();

    UFUNCTION()
        TArray<FString> GetSetupOrderArrayString();     // TODO implement

    UFUNCTION()
        TArray<int> GetCurrentSetup();

    


private:
    UPROPERTY()
        TArray<FSFStudySetting> Settings;

    UPROPERTY()
        TArray<FString> LevelNames;


    // Repititions
    UPROPERTY()
        int NumberOfRepetitions = 1;

    UPROPERTY()
        TEnumAsByte<EPhaseRepetitionType> TypeOfRepetition = EPhaseRepetitionType::SameOrder;


    // Mixing Stuff
    UPROPERTY()
        TEnumAsByte<EMixingSetupOrder> TypeOfMixing = EMixingSetupOrder::RandomSetupOrder;


    // Setups
    UPROPERTY()
        TArray<int> Order;          // TODO macht gar nichts ??

    TArray<TArray<int>> Order2D;

    UPROPERTY()
        TArray<int> UpcomingSetup;  

    UPROPERTY()
        TArray<int> CurrentSetup;   

    UPROPERTY()
        int CurrentSetupIdx = -1;

    UPROPERTY()
        FString UpcomingLevelName;  

    UPROPERTY()
        int NumberOfSetups;

    // Spawn on Level
    UPROPERTY()
        TArray<UClass*> SpawnInThisPhaseCpp;

    UPROPERTY()
        TArray<FSFClassOfBlueprintActor> SpawnInThisPhaseBlueprint;





public:

};
