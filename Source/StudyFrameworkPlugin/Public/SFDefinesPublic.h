#pragma once

#include "CoreMinimal.h"
#include "Class.h"

#include "SFDefinesPublic.generated.h"



USTRUCT()
struct FSFClassOfBlueprintActor
{
	GENERATED_BODY()

public:
    UPROPERTY()
	    FString Path;
	UPROPERTY()
	    FString ClassName;
};

UENUM()
enum EMixingSetupOrder                // Hier nur withinMap first? Rename into Repition order
{
    RandomSetupOrder = 0        UMETA(DisplayName = "Random"),
    WithinMapFirst = 1          UMETA(DisplayName = "WithinMapFirst")
};

UENUM()
enum EPhaseRepetitionType                // Hier nur withinMap first? Rename into Repition order
{
    SameOrder = 0               UMETA(DisplayName = "Same Order"),
    DifferentOrder = 1          UMETA(DisplayName = "Different Order")
};

// TODO Maybe better in Private Defines?
UENUM()
enum EFadeState
{
    NotFading = 0               UMETA(DisplayName = "Not Fading"),
    FadingOut = 1               UMETA(DisplayName = "Fading Out"),
    WaitForLevelLoaded = 2      UMETA(DisplayName = "Wait for Level Loaded"),
    WaitForTimerFadedOut = 3    UMETA(DisplayName = "Wait for Timer Faded Out"),
    FadingIn = 4                UMETA(DisplayName = "Fading In")
};

DECLARE_DELEGATE_OneParam(FFactorChangeDelegate, int);


USTRUCT()
struct FSFStudySetting
{
	GENERATED_BODY()

	FString			Name		= "";

    int			Count		= 1;

    FFactorChangeDelegate	Delegate;

    // // TODO struct operators needed?
	// bool operator==(FSFStudySetting const & Other) const {
	// 	return this->Name == Other.Name;
	// }
    // 
	// bool operator==(FString const & Other) const {
	// 	return this->Name == Other;
	// }
};


