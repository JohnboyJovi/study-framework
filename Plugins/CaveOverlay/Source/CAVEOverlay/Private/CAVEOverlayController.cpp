// Fill out your copyright notice in the Description page of Project Settings.

#include "CAVEOverlayController.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorOverlayData.h"
#include "IDisplayCluster.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#include "Components/InputComponent.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "IDisplayClusterGameManager.h"
#include "IDisplayClusterConfigManager.h"
#include "UObject/ConstructorHelpers.h"
#include <array>
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DisplayClusterExtensions/Public/VirtualRealityPawn.h"
#include "DisplayCluster/Public/DisplayClusterSceneComponent.h"

DEFINE_LOG_CATEGORY(LogCAVEOverlay);

template <std::size_t S>
bool ContainsFString(const std::array<FString, S>& Array, const FString& Entry)
{
	for (FString Current_Entry : Array)
	{
		if (Current_Entry.Equals(Entry, ESearchCase::IgnoreCase)) return true;
	}
	return false;
}

UStaticMeshComponent* ACAVEOverlayController::CreateMeshComponent(const FName& Name, UStaticMesh* Mesh, USceneComponent* Parent)
{
	UStaticMeshComponent* Result = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Result->SetStaticMesh(Mesh);
	Result->SetupAttachment(Parent);
	Result->SetVisibility(false);
	return Result;
}

template <typename T>
bool LoadAsset(const FString& Path, T* & Result)
{
	ConstructorHelpers::FObjectFinder<T> Loader(*Path);
	Result = Loader.Object;
	if (!Loader.Succeeded()) UE_LOG(LogCAVEOverlay, Error, TEXT("Could not find %s. Have you renamed it?"), *Path);
	return Loader.Succeeded();
}

// Sets default values
ACAVEOverlayController::ACAVEOverlayController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bAllowTickBeforeBeginPlay = false;
	AutoReceiveInput = EAutoReceiveInput::Player0;

	ConstructorHelpers::FClassFinder<UDoorOverlayData> WidgetClassFinder(TEXT("Blueprint'/CAVEOverlay/DoorOverlay'"));
	if (WidgetClassFinder.Succeeded())
	{
		Overlay_Class = WidgetClassFinder.Class;
	}
	else
	{
		UE_LOG(LogCAVEOverlay, Error, TEXT("Could not find the DoorOverlay class. Have you renamed it?"));
	}

	//Creation of sub-components
	Root = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	SetRootComponent(Root);
	Tape_Root = CreateDefaultSubobject<USceneComponent>("TapeRoot");
	Sign_Root = CreateDefaultSubobject<USceneComponent>("SignRoot");
	Tape_Root->SetupAttachment(Root);
	Sign_Root->SetupAttachment(Root);

	//Loading of Materials and Meshes
	LoadAsset("/CAVEOverlay/Stripes", Tape_Material);
	LoadAsset("/CAVEOverlay/StopMaterial", Sign_Material);
	LoadAsset("/CAVEOverlay/Plane", Plane_Mesh_);

	Tape_Negative_Y = CreateMeshComponent("TapeNegY", Plane_Mesh_, Tape_Root);
	Tape_Negative_X = CreateMeshComponent("TapeNegX", Plane_Mesh_, Tape_Root);
	Tape_Positive_Y = CreateMeshComponent("TapePosY", Plane_Mesh_, Tape_Root);
	Tape_Positive_X = CreateMeshComponent("TapePosX", Plane_Mesh_, Tape_Root);

	Sign_Negative_Y = CreateMeshComponent("SignNegY", Plane_Mesh_, Sign_Root);
	Sign_Negative_X = CreateMeshComponent("SignNegX", Plane_Mesh_, Sign_Root);
	Sign_Positive_Y = CreateMeshComponent("SignPosY", Plane_Mesh_, Sign_Root);
	Sign_Positive_X = CreateMeshComponent("SignPosX", Plane_Mesh_, Sign_Root);

	//Set initial Position, Rotation and Scale of Tape
	Tape_Negative_Y->SetRelativeLocationAndRotation(FVector(0, -Wall_Distance, 0), FRotator(0, 0, 90));
	Tape_Positive_Y->SetRelativeLocationAndRotation(FVector(0, +Wall_Distance, 0), FRotator(0, 180, 90));
	Tape_Negative_X->SetRelativeLocationAndRotation(FVector(-Wall_Distance, 0, 0), FRotator(0, -90, 90));
	Tape_Positive_X->SetRelativeLocationAndRotation(FVector(+Wall_Distance, 0, 0), FRotator(0, 90, 90));

	Tape_Negative_Y->SetRelativeScale3D(FVector(Wall_Distance / 100 * 2, 0.15, 1));
	Tape_Positive_Y->SetRelativeScale3D(FVector(Wall_Distance / 100 * 2, 0.15, 1));
	Tape_Negative_X->SetRelativeScale3D(FVector(Wall_Distance / 100 * 2, 0.15, 1));
	Tape_Positive_X->SetRelativeScale3D(FVector(Wall_Distance / 100 * 2, 0.15, 1));

	//Set initial Position, Rotation and Scale of Signs
	Sign_Negative_Y->SetRelativeLocationAndRotation(FVector(0, -Wall_Distance, 0), FRotator(0, 0, 90));
	Sign_Positive_Y->SetRelativeLocationAndRotation(FVector(0, +Wall_Distance, 0), FRotator(0, 180, 90));
	Sign_Negative_X->SetRelativeLocationAndRotation(FVector(-Wall_Distance, 0, 0), FRotator(0, -90, 90));
	Sign_Positive_X->SetRelativeLocationAndRotation(FVector(+Wall_Distance, 0, 0), FRotator(0, 90, 90));

	Sign_Negative_Y->SetRelativeScale3D(FVector(0.5f));
	Sign_Positive_Y->SetRelativeScale3D(FVector(0.5f));
	Sign_Negative_X->SetRelativeScale3D(FVector(0.5f));
	Sign_Positive_X->SetRelativeScale3D(FVector(0.5f));
}

void ACAVEOverlayController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//Create dynamic materials in runtime
	Tape_Material_Dynamic_ = UMaterialInstanceDynamic::Create(Tape_Material, Tape_Root);
	Sign_Material_Dynamic_ = UMaterialInstanceDynamic::Create(Sign_Material, Sign_Root);

	Tape_Negative_Y->SetMaterial(0, Tape_Material_Dynamic_);
	Tape_Negative_X->SetMaterial(0, Tape_Material_Dynamic_);
	Tape_Positive_Y->SetMaterial(0, Tape_Material_Dynamic_);
	Tape_Positive_X->SetMaterial(0, Tape_Material_Dynamic_);

	Sign_Negative_Y->SetMaterial(0, Sign_Material_Dynamic_);
	Sign_Negative_X->SetMaterial(0, Sign_Material_Dynamic_);
	Sign_Positive_Y->SetMaterial(0, Sign_Material_Dynamic_);
	Sign_Positive_X->SetMaterial(0, Sign_Material_Dynamic_);
}

void ACAVEOverlayController::CycleDoorType()
{
	Door_Current_Mode = static_cast<EDoor_Mode>((Door_Current_Mode + 1) % DOOR_NUM_MODES);

	IDisplayClusterClusterManager* const Manager = IDisplayCluster::Get().GetClusterMgr();
	if (Manager)
	{
		FDisplayClusterClusterEvent cluster_event;
		cluster_event.Name = "CAVEOverlay Change Door to " + Door_Mode_Names[Door_Current_Mode];
		cluster_event.Type = "DoorChange";
		cluster_event.Category = "CAVEOverlay";
		cluster_event.Parameters.Add("NewDoorState", FString::FromInt(Door_Current_Mode));
		Manager->EmitClusterEvent(cluster_event, true);
	}
}

void ACAVEOverlayController::HandleClusterEvent(const FDisplayClusterClusterEvent& Event)
{
	if (Event.Category.Equals("CAVEOverlay") && Event.Type.Equals("DoorChange") && Event.Parameters.Contains("NewDoorState"))
	{
		SetDoorMode(static_cast<EDoor_Mode>(FCString::Atoi(*Event.Parameters["NewDoorState"])));
	}
}

void ACAVEOverlayController::SetDoorMode(EDoor_Mode NewMode)
{
	Door_Current_Mode = NewMode;
	switch (Door_Current_Mode)
	{
	case EDoor_Mode::DOOR_DEBUG:
	case EDoor_Mode::DOOR_PARTIALLY_OPEN:
		Door_Current_Opening_Width_Absolute = Door_Opening_Width_Absolute;
		if (Screen_Type == SCREEN_DOOR) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (Screen_Type == SCREEN_DOOR_PARTIAL) Overlay->BlackBox->SetRenderScale(FVector2D(Door_Opening_Width_Relative, 1));
		if (Screen_Type == SCREEN_MASTER) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		Overlay->BlackBox->SetVisibility(ESlateVisibility::Visible);
		break;
	case EDoor_Mode::DOOR_OPEN:
		Door_Current_Opening_Width_Absolute = Wall_Distance * 2;
		if (Screen_Type == SCREEN_DOOR) Overlay->BlackBox->SetRenderScale(FVector2D(1, 1));
		if (Screen_Type == SCREEN_DOOR_PARTIAL) Overlay->BlackBox->SetRenderScale(FVector2D(1, 1));
		if (Screen_Type == SCREEN_MASTER) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		Overlay->BlackBox->SetVisibility(ESlateVisibility::Visible);
		break;
	case EDoor_Mode::DOOR_CLOSED:
		Door_Current_Opening_Width_Absolute = 0;
		if (Screen_Type == SCREEN_DOOR) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (Screen_Type == SCREEN_DOOR_PARTIAL) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (Screen_Type == SCREEN_MASTER) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		Overlay->BlackBox->SetVisibility(ESlateVisibility::Hidden);
		break;
	default: ;
	}
	if (Screen_Type == SCREEN_NORMAL) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1)); //no overlay

	UE_LOG(LogCAVEOverlay, Log, TEXT("Switched door state to '%s'. New opening width is %f."), *Door_Mode_Names[Door_Current_Mode], Door_Current_Opening_Width_Absolute);

	if (Screen_Type == SCREEN_MASTER)
	{
		Overlay->CornerText->SetText(FText::FromString(Door_Mode_Names[Door_Current_Mode]));
	}
}

// Called when the game starts or when spawned
void ACAVEOverlayController::BeginPlay()
{
	Super::BeginPlay();

	//Read situation
	if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster)
	{
		auto ClusterConfigCustom = IDisplayCluster::Get().GetConfigMgr()->GetConfigCustom();
		bCAVE_Mode = ClusterConfigCustom.Args.Contains("Hardware_Platform") && ClusterConfigCustom.Args.Find("Hardware_Platform")->Equals("aixcave", ESearchCase::IgnoreCase);
	}

	if (!bCAVE_Mode) return; // Not our business

	//Input config
	InputComponent->BindKey(EKeys::F10, EInputEvent::IE_Pressed, this, &ACAVEOverlayController::CycleDoorType);
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && !ClusterEventListenerDelegate.IsBound())
	{
		ClusterEventListenerDelegate = FOnClusterEventListener::CreateUObject(this, &ACAVEOverlayController::HandleClusterEvent);
		ClusterManager->AddClusterEventListener(ClusterEventListenerDelegate);
	}

	//Determine the screen-type for later usage
	if (IDisplayCluster::Get().GetClusterMgr()->GetNodeId().Equals(Screen_Main, ESearchCase::IgnoreCase))
	{
		Screen_Type = SCREEN_MASTER;
	}
	else if (ContainsFString(Screens_Door, IDisplayCluster::Get().GetClusterMgr()->GetNodeId()))
	{
		Screen_Type = SCREEN_DOOR;
	}
	else if (ContainsFString(Screens_Door_Partial, IDisplayCluster::Get().GetClusterMgr()->GetNodeId()))
	{
		Screen_Type = SCREEN_DOOR_PARTIAL;
	}
	else
	{
		Screen_Type = SCREEN_NORMAL;
	}

	Overlay = CreateWidget<UDoorOverlayData>(GetWorld()->GetFirstPlayerController(), Overlay_Class);
	Overlay->AddToViewport(0);
	SetDoorMode(Door_Current_Mode);
	Overlay->CornerText->SetText(FText::FromString("")); //Set Text to "" until someone presses the key for the first time

	Player_Pawn = Cast<AVirtualRealityPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
	RefreshPawnComponents();

	if (!bAttached && Cave_Origin)
	{
		AttachToComponent(Cave_Origin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bAttached = true;
	}
}

void ACAVEOverlayController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && ClusterEventListenerDelegate.IsBound())
	{
		ClusterManager->RemoveClusterEventListener(ClusterEventListenerDelegate);
	}

	Super::EndPlay(EndPlayReason);
}

float ACAVEOverlayController::CalculateOpacityFromPosition(FVector Position)
{
	return FMath::Max(
		FMath::Clamp((FMath::Abs(Position.X) - (Wall_Distance - Wall_Close_Distance)) / Wall_Fade_Distance, 0.0f, 1.0f),
		FMath::Clamp((FMath::Abs(Position.Y) - (Wall_Distance - Wall_Close_Distance)) / Wall_Fade_Distance, 0.0f, 1.0f)
	);
}

bool ACAVEOverlayController::PositionInDoorOpening(FVector Position)
{
	return FMath::IsWithinInclusive(-Position.X, Wall_Distance + 10 - 20 - Wall_Close_Distance, Wall_Distance + 10) //Overlap both sides 10cm
		&& FMath::IsWithinInclusive(-Position.Y, Wall_Distance + 10 - Door_Current_Opening_Width_Absolute, Wall_Distance + 10); //Overlap one side 10cm
}

void ACAVEOverlayController::RefreshPawnComponents()
{
	Cave_Origin = Player_Pawn->GetTrackingOriginComponent();
	Shutter_Glasses = Player_Pawn->GetHeadComponent();
}

// Called every frame
void ACAVEOverlayController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bCAVE_Mode) return; // Not our business

	if (!Cave_Origin || !Shutter_Glasses)
	{
		RefreshPawnComponents();
	}

	if (!bAttached && Cave_Origin)
	{
		AttachToComponent(Cave_Origin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bAttached = true;
	}

	//FPS Counter
	if (Overlay)
	{
		if (Door_Current_Mode == EDoor_Mode::DOOR_DEBUG && ContainsFString(Screens_FPS, IDisplayCluster::Get().GetClusterMgr()->GetNodeId()))
		{
			Overlay->FPS->SetText(FText::FromString(FString::Printf(TEXT("FPS: %.1f"), 1.0f / DeltaTime)));
		}
		else
		{
			Overlay->FPS->SetText(FText::FromString(""));
		}
	}

	if (!Shutter_Glasses) return; //Display Cluster not initialized

	//Tape Logic
	FVector Shutter_Position = Shutter_Glasses->GetRelativeTransform().GetLocation();
	bool bOverlay_Visible = FMath::IsWithinInclusive(Shutter_Position.GetAbsMax(), Wall_Distance - Wall_Close_Distance, Wall_Distance);

	if (bOverlay_Visible && !PositionInDoorOpening(Shutter_Position))
	{
		Tape_Root->SetVisibility(true, true);
		Tape_Root->SetRelativeLocation(Shutter_Position * FVector(0, 0, 1)); //Only apply Z

		float Tape_Opacity = CalculateOpacityFromPosition(Shutter_Position);
		Tape_Material_Dynamic_->SetScalarParameterValue("BarrierOpacity", Tape_Opacity);

		if (FMath::IsWithin(FVector2D(Shutter_Position).GetAbsMax(), Wall_Distance - Wall_Warning_Distance, Wall_Distance))
		{
			//in warning distance == red tape
			Tape_Material_Dynamic_->SetVectorParameterValue("StripeColor", FVector(1, 0, 0));
		}
		else
		{
			Tape_Material_Dynamic_->SetVectorParameterValue("StripeColor", FVector(1, 1, 0));
		}
	}
	else
	{
		Tape_Root->SetVisibility(false, true);
	}

	//Sign Logic
	UDisplayClusterSceneComponent* Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById(TEXT("flystick"));
	if (Flystick)
	{
		FVector Flystick_Position = Flystick->GetRelativeTransform().GetLocation();
		bool bFlystick_In_Door = PositionInDoorOpening(Flystick_Position);
		float Sign_Opacity = CalculateOpacityFromPosition(Flystick_Position);

		Sign_Negative_X->SetRelativeLocation(FVector(-Wall_Distance, Flystick_Position.Y, Flystick_Position.Z));
		Sign_Negative_Y->SetRelativeLocation(FVector(Flystick_Position.X, -Wall_Distance, Flystick_Position.Z));
		Sign_Positive_X->SetRelativeLocation(FVector(+Wall_Distance, Flystick_Position.Y, Flystick_Position.Z));
		Sign_Positive_Y->SetRelativeLocation(FVector(Flystick_Position.X, +Wall_Distance, Flystick_Position.Z));

		Sign_Negative_X->SetVisibility(FMath::IsWithin(-Flystick_Position.X, Wall_Distance - Wall_Close_Distance, Wall_Distance) && !bFlystick_In_Door);
		Sign_Negative_Y->SetVisibility(FMath::IsWithin(-Flystick_Position.Y, Wall_Distance - Wall_Close_Distance, Wall_Distance) && !bFlystick_In_Door);
		Sign_Positive_X->SetVisibility(FMath::IsWithin(+Flystick_Position.X, Wall_Distance - Wall_Close_Distance, Wall_Distance) && !bFlystick_In_Door);
		Sign_Positive_Y->SetVisibility(FMath::IsWithin(+Flystick_Position.Y, Wall_Distance - Wall_Close_Distance, Wall_Distance) && !bFlystick_In_Door);

		Sign_Material_Dynamic_->SetScalarParameterValue("SignOpacity", Sign_Opacity);
	}
	else
	{
		Sign_Negative_X->SetVisibility(false);
		Sign_Negative_Y->SetVisibility(false);
		Sign_Positive_X->SetVisibility(false);
		Sign_Positive_Y->SetVisibility(false);
	}
}
