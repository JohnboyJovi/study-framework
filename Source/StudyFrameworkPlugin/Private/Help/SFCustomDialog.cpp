// Copyright Epic Games, Inc. All Rights Reserved.
//This file is changed from an Unreal Engine file

#include "Help/SFCustomDialog.h"

#include "HAL/PlatformApplicationMisc.h"

//#include "EditorStyleSet.h"
//#include "Framework/Docking/TabManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Logging/LogMacros.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"

DEFINE_LOG_CATEGORY_STATIC(LogCustomDialog, Log, All);

void SFCustomDialog::Construct(const FArguments& InArgs)
{
	UE_LOG(LogCustomDialog, Log, TEXT("Dialog displayed:"), *InArgs._Title.ToString());

	check(InArgs._Buttons.Num() > 0);

	OnClosed = InArgs._OnClosed;

	TSharedPtr<SHorizontalBox> ContentBox;
	TSharedPtr<SHorizontalBox> ButtonBox;

	SWindow::Construct(SWindow::FArguments()
		.Title(InArgs._Title)
		.SizingRule(ESizingRule::Autosized)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		[
			SNew(SBorder)
			.Padding(4.f)
			//.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder")) //!!!
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(ContentBox, SHorizontalBox)
		]
	+ SVerticalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoHeight()
		[
			SAssignNew(ButtonBox, SHorizontalBox)
		]
		]
		]);

	if (InArgs._IconBrush.IsValid())
	{
		const FSlateBrush* ImageBrush = nullptr;// FEditorStyle::GetBrush(InArgs._IconBrush); //!!!
		if (ImageBrush != nullptr)
		{
			ContentBox->AddSlot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				.Padding(0, 0, 8, 0)
				[
					SNew(SImage)
					.Image(ImageBrush)
				];
		}
	}

	if (InArgs._UseScrollBox)
	{
		ContentBox->AddSlot()
			[
				SNew(SBox)
				.MaxDesiredHeight(InArgs._ScrollBoxMaxHeight)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
			[
				InArgs._DialogContent.ToSharedRef()
			]
			]
			];
	}
	else
	{
		ContentBox->AddSlot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				InArgs._DialogContent.ToSharedRef()
			];
	}

	ButtonBox->AddSlot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(FVector2D(20.0f, 1.0f))
		];

	TSharedPtr<SUniformGridPanel> ButtonPanel;

	ButtonBox->AddSlot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		[
			SAssignNew(ButtonPanel, SUniformGridPanel)
			//.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding")) //!!!
			//.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth")) //!!!
			//.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight")) //!!!
		];

	for (int32 i = 0; i < InArgs._Buttons.Num(); ++i)
	{
		const FButton& Button = InArgs._Buttons[i];

		ButtonPanel->AddSlot(ButtonPanel->GetChildren()->Num(), 0)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateSP(this, &SFCustomDialog::OnButtonClicked, Button.OnClicked, i))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(Button.ButtonText)
			]
			]
			];
	}
}

int32 SFCustomDialog::ShowModal()
{
	FSlateApplication::Get().AddModalWindow(StaticCastSharedRef<SWindow>(this->AsShared()), FGlobalTabmanager::Get()->GetRootWindow());

	return LastPressedButton;
}

void SFCustomDialog::Show()
{
	TSharedRef<SWindow> Window = FSlateApplication::Get().AddWindow(StaticCastSharedRef<SWindow>(this->AsShared()), true);

	if (OnClosed.IsBound())
	{
		Window->GetOnWindowClosedEvent().AddLambda([this](const TSharedRef<SWindow>& Window) { OnClosed.Execute(); });
	}
}

/** Handle the button being clicked */
FReply SFCustomDialog::OnButtonClicked(FSimpleDelegate OnClicked, int32 ButtonIndex)
{
	LastPressedButton = ButtonIndex;

	FSlateApplication::Get().RequestDestroyWindow(StaticCastSharedRef<SWindow>(this->AsShared()));

	OnClicked.ExecuteIfBound();
	return FReply::Handled();
}
