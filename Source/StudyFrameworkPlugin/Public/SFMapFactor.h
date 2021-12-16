#pragma once

#include "CoreMinimal.h"

#include "SFStudyFactor.h"

#include "SFMapFactor.generated.h"

UCLASS(EditInlineNew)
class STUDYFRAMEWORKPLUGIN_API USFMapFactor : public USFStudyFactor
{
	GENERATED_BODY()

public:
	USFMapFactor();

	TSharedPtr<FJsonObject> GetAsJson() const override;
};
