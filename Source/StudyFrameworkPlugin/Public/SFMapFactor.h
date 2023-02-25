#pragma once

#include "CoreMinimal.h"

#include "SFStudyFactor.h"

#include "SFMapFactor.generated.h"

/*
 * This is a special Study Factor representing a maps/levels which are faded to
 */
UCLASS(EditInlineNew)
class STUDYFRAMEWORKPLUGIN_API USFMapFactor : public USFStudyFactor
{
	GENERATED_BODY()

public:
	USFMapFactor();

	virtual TSharedPtr<FJsonObject> GetAsJson() const override;
	virtual void FromJson(TSharedPtr<FJsonObject> Json) override;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<TAssetPtr<UWorld>> Maps;
};
