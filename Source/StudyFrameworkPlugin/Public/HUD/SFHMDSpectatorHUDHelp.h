#pragma once
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"

#include "SFHMDSpectatorHUDHelp.generated.h"

// this is copied and adapted from: https://calben.github.io/UnrealVrSpectatorControl/#/
// which links to the repo: https://github.com/calben/UnrealVRSpectatorUtilitiesPlugin/

UCLASS()
class ASFHMDSpectatorHUDHelp : public AActor
{
	GENERATED_BODY()

public:
	ASFHMDSpectatorHUDHelp();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UUserWidget* CreateWidget(TSubclassOf<UUserWidget> WidgetClass);
	const UWidgetComponent* GetWidgetComponent() const;

	/**
	* Calculates the expected cursor position from a viewport of some size to a widget of some size.
	*
	* @param PlayerController The player controller reference from which to get the mouse position.
	* @param WidgetSize The size of the widget for which the cursor position should be calculated.
	* @return Returns the "absolute" location for the widget as a FVector2D offset from the widget origin of 0,0 (top left corner).
	*/
	UFUNCTION()
	static const FVector2D GetAbsoluteLocationForCursorWidgetFromMousePosition(
		class APlayerController* PlayerController, const FVector2D WidgetSize = FVector2D(1920.f, 1080.f));

	/**
	* Retargets a cursor's absolute position to a 3D widget given that in order for widget interaction to work correctly, the widget component should have a pivot of (0.5, 0.5).
	*
	* @param AbsoluteCursorLocation The position of the cursor as an offset from the widget origin.
	* @param TargetResolution The target resolution of the widget.
	* @return The cursor location corrected for the pivot offset.
	*/
	UFUNCTION()
	static const FVector2D OffsetCursorWidgetFromMouseLocationForMiddlePivot(
		const FVector2D AbsoluteCursorLocation, const FVector2D TargetResolution = FVector2D(1920.f, 1080.f));

	/**
	* Gets the spectator display resolution for both in editor and in game
	*
	* @return The display resolution.
	*/
	UFUNCTION()
	static const FVector2D GetSpectatorDisplayResolution();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* WidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetInteractionComponent* InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D DefaultDrawSize = FVector2D(1920, 1080);
};
