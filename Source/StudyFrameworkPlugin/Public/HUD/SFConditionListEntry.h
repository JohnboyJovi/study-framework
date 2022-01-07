// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "SFCondition.h"
#include "Components/Border.h"

#include "SFConditionListEntry.generated.h"

/**
 * 
 */
UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFConditionListEntry : public UUserWidget
{
	GENERATED_BODY()
public:

	void FillWithCondition(const USFCondition* Condition);
	void FillAsPhaseHeader(const USFCondition* Condition);

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UButton* GoToButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Phase;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Map;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Text0;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Text1;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Text2;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Text3;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Text4;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Text5;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UTextBlock* Time;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget)) UBorder* BackgroundColor;

	UPROPERTY(EditAnywhere) FLinearColor DefaultColor;
	UPROPERTY(EditAnywhere) FLinearColor HeaderColor;
	UPROPERTY(EditAnywhere) FLinearColor ActiveColor;
	UPROPERTY(EditAnywhere) FLinearColor DoneColor;

	UPROPERTY(BlueprintReadWrite) bool IsHeader = false;
	UPROPERTY(BlueprintReadWrite) bool IsDone = false;
	UPROPERTY(BlueprintReadWrite) bool IsActive = false;

	UFUNCTION(BlueprintCallable) void GoTo();
protected:
	UFUNCTION(BlueprintCallable) void UpdateData();

private:
	UPROPERTY() const USFCondition* Condition;

	void FillTextsHelper(const TArray<FString>& Data);
	//the USFDependentVariable is owned by the Condition, so it is not garbage-collected
	TMap<int, USFDependentVariable*> TextBlockIdToDependentVar;
};
