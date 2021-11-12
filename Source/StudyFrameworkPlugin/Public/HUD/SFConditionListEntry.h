// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "SFConditionListEntry.generated.h"

/**
 * 
 */
UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFConditionListEntry : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Phase;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Map;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UButton* GoToButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) TArray<UTextBlock*> Factors;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) TArray<UTextBlock*> DependenVars;


	UPROPERTY(BlueprintReadWrite) bool IsHeader = false;
	UPROPERTY(BlueprintReadWrite) bool IsDone = false;
	UPROPERTY(BlueprintReadWrite) bool IsActive = false;

	UFUNCTION(BlueprintCallable) void GoTo();
};
