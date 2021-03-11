// Fill out your copyright notice in the Description page of Project Settings.


#include "SFWidget.h"

#include "SFUtils.h"


USFWidget::USFWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void USFWidget::NativeConstruct()
{
    Super::NativeConstruct();
}


void USFWidget::SetJsonData(TSharedPtr<FJsonObject> Data)
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

void USFWidget::SetText(FString Text)
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

void USFWidget::ClearWidget()
{
    if (TextBox)
    {
        TextBox->SetText(FText::FromString(""));
        TextBox->SetVisibility(ESlateVisibility::Hidden);
    }
}
