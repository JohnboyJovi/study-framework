// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"

// #include "JsonObject.h"
// #include "SharedPointer.h"


#include "SFWidget.generated.h"

/**
 * 
 */
UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    USFWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    
    void SetJsonData(TSharedPtr<FJsonObject> Data);
    void SetText(FString Text);

    void ClearWidget();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBox;
	
};
