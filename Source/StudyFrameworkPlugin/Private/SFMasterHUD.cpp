// Fill out your copyright notice in the Description page of Project Settings.


#include "SFMasterHUD.h"
#include "SFGlobalFadeGameViewportClient.h"

ASFMasterHUD::ASFMasterHUD()
{

}

void ASFMasterHUD::DrawHUD()
{
	DrawBackground();
    Super::DrawHUD();
}

void ASFMasterHUD::BeginPlay()
{
    Super::BeginPlay();
    if (SFWidgetClass)
    {
        SFWidget = CreateWidget<USFWidget>(GetWorld(), SFWidgetClass);
        
        if (SFWidget)
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

void ASFMasterHUD::SetBackgroundColor(FLinearColor Color)
{
	BackgroundColor = Color;
}

void ASFMasterHUD::DrawBackground()
{
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	DrawRect(BackgroundColor, 0.0f, 0.0f, ViewportSize.X, ViewportSize.Y);
}
