#pragma once

#include "CoreMinimal.h"

#include "SFIndependentVariable.generated.h"

UENUM()
enum class EValType {
	TEXT = 0 UMETA(DisplayName = "Text"),
	MULTIPLECHOICE = 1 UMETA(DisplayName = "Multiple Choice")
};

UCLASS(EditInlineNew, CollapseCategories)
class STUDYFRAMEWORKPLUGIN_API USFIndependentVariable : public UObject
{
	GENERATED_BODY()

public:
	USFIndependentVariable();

	TSharedPtr<FJsonObject> GetAsJson() const;
	void FromJson(TSharedPtr<FJsonObject> Json);

	FString GetValTypeAsString() const;
	void SetValTypeFromString(const FString& Str);

#if WITH_EDITOR
	//check what properties can be changed/used dependent on other properties
	virtual bool CanEditChange(const FProperty* InProperty) const;
#endif

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString Name = "undefined";

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EValType ValueType = EValType::TEXT;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bAskedAtBeginning = true;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString Prompt = "";

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FString> Options;

};
