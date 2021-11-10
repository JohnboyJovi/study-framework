#pragma once

#include "CoreMinimal.h"

#include "SFDefines.generated.h"


// This struct is used to identify blueprints to spawn
// ClassName: The name of the blueprint as shown in the Editor
// Path: where the blueprint is stored
//     for blueprints in the game's Content dir: use /Game/[path_relative_to_Content_dir]

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
enum EMixingSetupOrder // Hier nur withinMap first? Rename into Repetion order
{
	RandomSetupOrder = 0 UMETA(DisplayName = "Random"),
	WithinMapFirst = 1 UMETA(DisplayName = "WithinMapFirst")
};

UENUM()
enum EPhaseRepetitionType // Hier nur withinMap first? Rename into Repition order
{
	SameOrder = 0 UMETA(DisplayName = "Same Order"),
	DifferentOrder = 1 UMETA(DisplayName = "Different Order")
};
