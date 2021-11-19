// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "HUD/SFHMDSpectatorHUDHelp.h"

#if WITH_EDITOR
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#endif

#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "HUD/SFHUDWidget.h"
#include "Pawn/VirtualRealityPawn.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Runtime/Engine/Public/Slate/SceneViewport.h"
#include "Runtime/Engine/Classes/Engine/GameEngine.h"

ASFHMDSpectatorHUDHelp::ASFHMDSpectatorHUDHelp()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Widget Component");
	WidgetComponent->SetupAttachment(Root);

	//WidgetComponent->SetRelativeRotation(FRotator(90,0,0));
	InteractionComponent = CreateDefaultSubobject<UWidgetInteractionComponent>("Interaction Component");
	InteractionComponent->SetupAttachment(WidgetComponent);
	//InteractionComponent->VirtualUserIndex=1;
	InteractionComponent->PointerIndex = 1;
	InteractionComponent->bShowDebug=true;
	InteractionComponent->InteractionDistance = 10.0f;
	InteractionComponent->SetRelativeRotation(FRotator(0,180,0));
}

void ASFHMDSpectatorHUDHelp::BeginPlay()
{
	Super::BeginPlay();
}


void ASFHMDSpectatorHUDHelp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//the widget needs to be always be facing away, so we do not see it in the HMD view
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	//AVirtualRealityPawn* Pawn = Cast<AVirtualRealityPawn>(PlayerController->AcknowledgedPawn);
	//FVector HeadPos = Pawn->Head->GetComponentLocation();
	//SetActorRotation(FQuat::FindBetweenNormals(FVector(0,0,1),(GetActorLocation()-HeadPos).GetSafeNormal()).Rotator());

	//Set cursor to the right place
	USFHUDWidget* HUDWidget = Cast<USFHUDWidget>(WidgetComponent->GetWidget());
	FVector2D CursorPos = GetAbsoluteLocationForCursorWidgetFromMousePosition(PlayerController, DefaultDrawSize);
	HUDWidget->SetCursorWidgetPosition(CursorPos);

	//Set widget interaction
	FVector2D RelativePos = OffsetCursorWidgetFromMouseLocationForMiddlePivot(CursorPos, DefaultDrawSize);
	InteractionComponent->SetRelativeLocation(FVector(1, RelativePos.X, RelativePos.Y));
	

	//clicking?
	if(PlayerController->WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		InteractionComponent->PressPointerKey(EKeys::RightMouseButton);
	}
	if(PlayerController->WasInputKeyJustReleased(EKeys::LeftMouseButton))
	{
		InteractionComponent->ReleasePointerKey(EKeys::RightMouseButton);
	}
	//InteractionComponent->PressPointerKey(EKeys::LeftMouseButton);
	//InteractionComponent->ReleasePointerKey(EKeys::LeftMouseButton);
}

UUserWidget* ASFHMDSpectatorHUDHelp::CreateWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	WidgetComponent->SetWidgetClass(WidgetClass);
	WidgetComponent->InitWidget();
	WidgetComponent->SetDrawSize(DefaultDrawSize);
	WidgetComponent->SetTickWhenOffscreen(true);
	WidgetComponent->SetCastShadow(false);
	//WidgetComponent->SetVisibility(false); //also not rendering into RenderTarget when used!
	return WidgetComponent->GetWidget();
}

const UWidgetComponent* ASFHMDSpectatorHUDHelp::GetWidgetComponent() const
{
	return WidgetComponent;
}


const FVector2D ASFHMDSpectatorHUDHelp::GetAbsoluteLocationForCursorWidgetFromMousePosition(
	APlayerController* PlayerController, const FVector2D WidgetSize)
{
	float mouse_x;
	float mouse_y;
	PlayerController->GetMousePosition(mouse_x, mouse_y);
	const FVector2D DisplaySize = GetSpectatorDisplayResolution();
	FVector2D MousePositionPercentage = FVector2D(mouse_x / DisplaySize.X, mouse_y / DisplaySize.Y);
	return MousePositionPercentage * WidgetSize;
}

const FVector2D ASFHMDSpectatorHUDHelp::OffsetCursorWidgetFromMouseLocationForMiddlePivot(
	const FVector2D AbsoluteCursorLocation, const FVector2D TargetResolution)
{
	FVector2D Result = TargetResolution / 2.f - AbsoluteCursorLocation;
	return Result;
}

const FVector2D ASFHMDSpectatorHUDHelp::GetSpectatorDisplayResolution()
{
	FSceneViewport* sceneViewport = nullptr;

	if (!GIsEditor)
	{
		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		sceneViewport = GameEngine->SceneViewport.Get();
	}
#if WITH_EDITOR
	else
	{
		UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEngine);
		FSceneViewport* PIEViewport = (FSceneViewport*)EditorEngine->GetPIEViewport();
		if (PIEViewport != nullptr && PIEViewport->IsStereoRenderingAllowed())
		{
			sceneViewport = PIEViewport;
		}
		else
		{
			FSceneViewport* EditorViewport = (FSceneViewport*)EditorEngine->GetActiveViewport();
			if (EditorViewport != nullptr && EditorViewport->IsStereoRenderingAllowed())
			{
				sceneViewport = EditorViewport;
			}
		}
	}
#endif
	if (sceneViewport != nullptr)
	{
		return sceneViewport->FindWindow()->GetSizeInScreen();
	}

	return FVector2D();
}
