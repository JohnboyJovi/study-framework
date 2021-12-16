#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"

#include "SFStudyFactor.generated.h"

UENUM()
enum class EFactorMixingOrder : uint8
{
	RandomOrder = 0 UMETA(DisplayName = "Random"),
	EnBlock = 1 UMETA(DisplayName = "All conditions with the same level of this factor will be shown en block")
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

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString FactorName = "undefined";

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FString> Levels;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EFactorMixingOrder MixingOrder = EFactorMixingOrder::RandomOrder;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EFactorType Type = EFactorType::Within;
};
