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


		// Create Viewport Widget
	TSharedPtr<SViewport> Viewport = SNew(SViewport)
		.IsEnabled(true)
		.EnableGammaCorrection(false)
		.ShowEffectWhenDisabled(false)
		.EnableBlending(true)
		.ToolTip(SNew(SToolTip).Text(FText::FromString("ExperimenterViewport")));


	// Create Scene Viewport
	//TSharedPtr<FSceneViewport> SceneViewport = MakeShareable(ASFHMDSpectatorHUDHelp::GetSceneViewport());

	// Assign SceneViewport to Viewport widget. It needed for rendering
	//Viewport->SetViewportInterface(SceneViewport.ToSharedRef());

	//SecondWindow->SetContent(Viewport.ToSharedRef());
}

void USFExperimenterWindow::DestroyWindow()
{
	SecondWindow->DestroyWindowImmediately();
	SecondWindow = nullptr;
}


