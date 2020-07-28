// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TextBlock.h"
#include "Border.h"
#include "DoorOverlayData.generated.h"

/**
 * 
 */
UCLASS()
class CAVEOVERLAY_API UDoorOverlayData : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//These declarations are magically bound to the UMG blueprints elements,
	//if they are named the same
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* CornerText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UBorder* BlackBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* FPS;
};
