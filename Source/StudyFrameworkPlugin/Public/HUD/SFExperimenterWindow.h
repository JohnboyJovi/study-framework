#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"


#include "SFExperimenterWindow.generated.h"


USTRUCT(BlueprintType)
struct FExperimenterViewConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bShowHUD = true;

	UPROPERTY(EditAnywhere)
	bool bShowConditionsPanelByDefault = false;

	UPROPERTY(EditAnywhere)
	bool bShowExperimenterViewInSecondWindow = false;

	UPROPERTY(EditAnywhere)
	FVector2D SecondWindowSize = FVector2D(1920,1080);

	UPROPERTY(EditAnywhere)
	FVector2D SecondWindowPos = FVector2D(1920,0);

	TSharedPtr<FJsonObject> GetAsJson() const;
	void FromJson(TSharedPtr<FJsonObject> Json);
};


UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFExperimenterWindow : public UObject
{
	GENERATED_BODY()

public:

	void CreateWindow(FExperimenterViewConfig Config);
	void DestroyWindow();
	void AddHUDWidget(TSharedRef<SWidget>& Widget);


private:

	TSharedPtr<SWindow> SecondWindow;
	TSharedPtr<SViewport> Viewport;
	SOverlay::FOverlaySlot* HUDWidgetOverlay;
};
