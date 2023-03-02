#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"

#include "SFStudyFactor.generated.h"

UENUM()
enum class EFactorMixingOrder : uint8
{
	RandomOrder = 0 UMETA(DisplayName = "Random: Using Latin Square to balance orders between participants"),
	EnBlock = 1 UMETA(DisplayName = "En Block: All conditions with the same level of this factor will be shown en block"),
	InOrder = 2 UMETA(DisplayName = "In Order: The levels of this factor well be kept in specified order")
};

UENUM()
enum class EFactorType : uint8
{
	Within = 0 UMETA(DisplayName = "Within: participants see all leveles"),
	Between = 1 UMETA(DisplayName = "Between: participants only see one level of this factor each")
};

UCLASS(EditInlineNew)
class STUDYFRAMEWORKPLUGIN_API USFStudyFactor : public UObject
{
	GENERATED_BODY()

public:
	USFStudyFactor();

	virtual TSharedPtr<FJsonObject> GetAsJson() const;
	virtual void FromJson(TSharedPtr<FJsonObject> Json);

#if WITH_EDITOR
	//check what properties can be changed/used dependent on other properties
	virtual bool CanEditChange(const FProperty* InProperty) const;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	//Name of this factor, used for identifying it and also for getting its level during execution
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString FactorName = "undefined";

	//the levels this factor can have (e.g., On, Off)
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FString> Levels;

	//how to mix this Factor
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EFactorMixingOrder MixingOrder = EFactorMixingOrder::RandomOrder;

	//Whether it is a within or between subjects factor
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EFactorType Type = EFactorType::Within;

	/*
	*	bNonCombined means this factor is just used to introduce randomness
	*	In contrast to normal factors it is not combined with the other factors in
	*	a n x m x ... design but run in parallel, e.g., to introduce some randomness
	*	to repetitions of the same conditions (of the other factors).
	*	Currently these are then randomized between all subjects using LatinSquare
	*	(and repeated in the same order if not enough levels).
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bNonCombined = false;

	static TArray<int> GenerateLatinSquareOrder(int OrderNr, int NrConditions);
};
