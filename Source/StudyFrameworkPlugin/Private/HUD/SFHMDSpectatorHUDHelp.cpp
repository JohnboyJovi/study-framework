// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "HUD/SFHMDSpectatorHUDHelp.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#endif

#include "IXRTrackingSystem.h"
#include "HUD/SFHUDWidget.h"
#include "Pawn/VirtualRealityPawn.h"
#include "GameFramework/PlayerController.h"
#include "Slate/SceneViewport.h"

ASFHMDSpectatorHUDHelp::ASFHMDSpectatorHUDHelp()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Widget Component");
	WidgetComponent->SetupAttachment(Root);

	InteractionComponent = CreateDefaultSubobject<UWidgetInteractionComponent>("Interaction Component");
	InteractionComponent->SetupAttachment(WidgetComponent);
	InteractionComponent->VirtualUserIndex = 1;
	//InteractionComponent->bShowDebug = true;
	InteractionComponent->InteractionDistance = 10.0f; //only 10 cm, because the interaction component is 1cm away from the widget
	InteractionComponent->SetRelativeRotation(FRotator(0, 180, 0));

	
}

void ASFHMDSpectatorHUDHelp::BeginPlay()
{
	Super::BeginPlay();

	UInputComponent* Input = GetWorld()->GetFirstPlayerController()->InputComponent;
	//potentially right-clicks could be used here, since the left click is currently also forwarded to the "Fire" Action
	Input->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ASFHMDSpectatorHUDHelp::OnMouseClicked);
	Input->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ASFHMDSpectatorHUDHelp::OnMouseReleased);

	//DrawSize = GetSpectatorDisplayResolution(); //as of now we always use 1080p since the HUD is designed for that
	if(WidgetComponent->GetWidget())
	{
		//if CreateWidget was already set, overwrite that
		WidgetComponent->SetDrawSize(DrawSize);
	}
}


void ASFHMDSpectatorHUDHelp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//the widget needs to be always be facing away, so we do not see it in the HMD view
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	AVirtualRealityPawn* Pawn = Cast<AVirtualRealityPawn>(PlayerController->AcknowledgedPawn);
	FVector HeadPos = Pawn->Head->GetComponentLocation();
	SetActorRotation(FQuat::FindBetweenNormals(FVector(0,0,1),(GetActorLocation()-HeadPos).GetSafeNormal()).Rotator());

	//Set cursor to the right place
	USFHUDWidget* HUDWidget = Cast<USFHUDWidget>(WidgetComponent->GetWidget());
	FVector2D CursorPos = GetAbsoluteLocationForCursorWidgetFromMousePosition(PlayerController, DrawSize);
	HUDWidget->SetCursorWidgetPosition(CursorPos);

	//Set widget interaction to the right spot
	FVector2D RelativePos = OffsetCursorWidgetFromMouseLocationForMiddlePivot(CursorPos, DrawSize);
	InteractionComponent->SetRelativeLocation(FVector(1, RelativePos.X, RelativePos.Y));
}

UUserWidget* ASFHMDSpectatorHUDHelp::CreateWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	WidgetComponent->SetWidgetClass(WidgetClass);
	WidgetComponent->InitWidget();
	WidgetComponent->SetDrawSize(DrawSize);
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

FSceneViewport* ASFHMDSpectatorHUDHelp::GetSceneViewport(bool bRequireStereo /*= falss*/)
{
	if (!GIsEditor)
	{
		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		return GameEngine->SceneViewport.Get();
	}
#if WITH_EDITOR
	else
	{
		UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEngine);
		FSceneViewport* PIEViewport = (FSceneViewport*)EditorEngine->GetPIEViewport();
		if (PIEViewport != nullptr && (!bRequireStereo || PIEViewport->IsStereoRenderingAllowed()))
		{
			return PIEViewport;
		}
		else
		{
			FSceneViewport* EditorViewport = (FSceneViewport*)EditorEngine->GetActiveViewport();
			if (EditorViewport != nullptr && (!bRequireStereo || EditorViewport->IsStereoRenderingAllowed()))
			{
				return EditorViewport;
			}
		}
	}
#endif
	return nullptr;
}

const FVector2D ASFHMDSpectatorHUDHelp::GetSpectatorDisplayResolution()
{
	FSceneViewport* sceneViewport = GetSceneViewport(true);
	if (sceneViewport != nullptr)
	{
		return sceneViewport->FindWindow()->GetSizeInScreen();
	}

	return FVector2D();
}

void ASFHMDSpectatorHUDHelp::OnMouseClicked()
{
	//We need to forward the clicks to the right user (see VirtualUserIndex=1 above)
	InteractionComponent->PressPointerKey(EKeys::LeftMouseButton);
}

void ASFHMDSpectatorHUDHelp::OnMouseReleased()
{
	//We need to forward the clicks to the right user (see VirtualUserIndex=1 above)
	InteractionComponent->ReleasePointerKey(EKeys::LeftMouseButton);
}

