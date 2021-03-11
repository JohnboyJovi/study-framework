// Fill out your copyright notice in the Description page of Project Settings.


#include "SFMasterHUD.h"

ASFMasterHUD::ASFMasterHUD()
{

}

void ASFMasterHUD::DrawHUD()
{
    Super::DrawHUD();
}

void ASFMasterHUD::BeginPlay()
{
    Super::BeginPlay();
    if (SFWidgetClass)
    {
        SFWidget = CreateWidget<USFWidget>(GetWorld(), SFWidgetClass);
        
        if (SFWidget != nullptr && SFWidget)
        {
            SFWidget->AddToViewport();
        }
    }
}

void ASFMasterHUD::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ASFMasterHUD::SetJsonData(TSharedPtr<FJsonObject> Data)
{
    if (SFWidget != nullptr && SFWidget)
    {
        SFWidget->SetJsonData(Data);
    }
}

void ASFMasterHUD::SetText(FString Text)
{
    if (SFWidget != nullptr && SFWidget)
    {
        SFWidget->SetText(Text);
    }
}

void ASFMasterHUD::ClearWidget()
{
    if (SFWidget != nullptr && SFWidget)
    {
        SFWidget->ClearWidget();
    }
}
