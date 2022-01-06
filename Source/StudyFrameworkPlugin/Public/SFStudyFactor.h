#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"

#include "SFStudyFactor.generated.h"

UENUM()
enum class EFactorMixingOrder : uint8
{
	RandomOrder = 0 UMETA(DisplayName = "Random: Using Latin Square to balance orders between participants"), 
	EnBlock = 1 UMETA(DisplayName = "En Block: All conditions with the same level of this factor will be shown en block")
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
	void FromJson(TSharedPtr<FJsonObject> Json);

	//Name of this factor, used for identifying it and also for getting its level during execution
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString FactorName = "undefined";

	//the levels this factor can have (e.g., On, Off)
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FString> Levels;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	//how to mix this Factor
	EFactorMixingOrder MixingOrder = EFactorMixingOrder::RandomOrder;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	//Whether it is a within or between subjects factor
	EFactorType Type = EFactorType::Within;

	static TArray<int> GenerateLatinSquareOrder(int ParticipantId, int NrConditions);
};
