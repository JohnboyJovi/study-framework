// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/SFExperimenterWindow.h"
#include "HUD/SFHMDSpectatorHUDHelp.h"

#include "Dom/JsonObject.h"
#include "Slate/SceneViewport.h"
#include "Widgets/SViewport.h"


TSharedPtr<FJsonObject> FExperimenterViewConfig::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	Json->SetBoolField("ShowHUD", bShowHUD);
	Json->SetBoolField("ShowConditionsPanelByDefault", bShowConditionsPanelByDefault);
	Json->SetBoolField("ShowExperimenterViewInSecondWindow", bShowExperimenterViewInSecondWindow);
	Json->SetNumberField("SecondWindowSizeX", SecondWindowSize.X);
	Json->SetNumberField("SecondWindowSizeY", SecondWindowSize.Y);
	Json->SetNumberField("SecondWindowPosX", SecondWindowPos.X);
	Json->SetNumberField("SecondWindowPosY", SecondWindowPos.Y);
	return Json;
}

void FExperimenterViewConfig::FromJson(TSharedPtr<FJsonObject> Json)
{

	bShowHUD = Json->GetBoolField("ShowHUD");
	bShowConditionsPanelByDefault = Json->GetBoolField("ShowConditionsPanelByDefault");
	bShowExperimenterViewInSecondWindow = Json->GetBoolField("ShowExperimenterViewInSecondWindow");
	SecondWindowSize = FVector2D(Json->GetNumberField("SecondWindowSizeX"), Json->GetNumberField("SecondWindowSizeY"));
	SecondWindowPos = FVector2D(Json->GetNumberField("SecondWindowPosX"), Json->GetNumberField("SecondWindowPosY"));
}

void USFExperimenterWindow::CreateWindow(FExperimenterViewConfig Config)
{
	SecondWindow = SNew(SWindow)
	.AutoCenter(EAutoCenter::None)
	.Title(FText::FromString(TEXT("Experimenter View")))
	.IsInitiallyMaximized(false)
	.ScreenPosition(Config.SecondWindowPos)
	.ClientSize(Config.SecondWindowSize)
	.CreateTitleBar(true)
	.SizingRule(ESizingRule::UserSized)
	.SupportsMaximize(true)
	.SupportsMinimize(true)
	.HasCloseButton(true);

	FSlateApplication & SlateApp = FSlateApplication::Get();

	SlateApp.AddWindow(SecondWindow.ToSharedRef(), true);

	// add the viewport of the primary window :-)
	SecondWindow->SetContent(ASFHMDSpectatorHUDHelp::GetSceneViewport()->GetViewportWidget().Pin().ToSharedRef());

	//add overlay where we attach HUD widget to later on
	HUDWidgetOverlay = &SecondWindow->AddOverlaySlot();
}

void USFExperimenterWindow::DestroyWindow()
{
	SecondWindow->RequestDestroyWindow();
	SecondWindow.Reset();
	Viewport.Reset();

}

void USFExperimenterWindow::AddHUDWidget(TSharedRef<SWidget>& Widget)
{
	HUDWidgetOverlay->DetachWidget();
	HUDWidgetOverlay->AttachWidget(Widget);
}


