// Fill out your copyright notice in the Description page of Project Settings.


#include "SFHUDWidget.h"

#include "SFUtils.h"


USFHUDWidget::USFWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void USFHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
}


void USFHUDWidget::SetJsonData(TSharedPtr<FJsonObject> Data)
{
    if (TextBox)
    {
        if (TextBox->Visibility == ESlateVisibility::Hidden)
        {
            TextBox->SetVisibility(ESlateVisibility::Visible);
        }
    
        TextBox->SetText(FText::FromString(FSFUtils::JsonToString(Data)));
    }
}

void USFHUDWidget::SetText(FString Text)
{
    if (TextBox)
    {
        if (TextBox->Visibility == ESlateVisibility::Hidden)
        {
            TextBox->SetVisibility(ESlateVisibility::Visible);
        }
    
        TextBox->SetText(FText::FromString(Text));
    }
}

void USFHUDWidget::ClearWidget()
{
    if (TextBox)
    {
        TextBox->SetText(FText::FromString(""));
        TextBox->SetVisibility(ESlateVisibility::Hidden);
    }
}
