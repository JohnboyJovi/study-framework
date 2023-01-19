// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"
#include "SFParticipant.h"
#include "Dialogs/CustomDialog.h"

class FJsonObject;

class FSFUtils
{
public:

	// Open Message Box with Text. if error is set, the window is titled ERROR //
	static void OpenMessageBox(FString Text, bool bError = false);
	// Open Dialog Popup with custom button options. Returns the index of the pressed button according to the Buttons parameter, -1 if there is another closed event.
	static int OpenCustomDialog(const FString& Title, const FString& Content, const TArray<FString>& Buttons);
	// Open Dialog Popup with a text input field. Returns 1 if Submit was clicked and OutText contains the text input, -1 if there is another closed event..
	static int OpenCustomDialogText(const FString& Title, const FString& Content, const FString& DefaultText, FString& OutText);


	static FString JsonToString(TSharedPtr<FJsonObject> Json);
	static TSharedPtr<FJsonObject> StringToJson(FString String);

	static void WriteJsonToFile(TSharedPtr<FJsonObject> Json, FString FilePath);
	static TSharedPtr<FJsonObject> ReadJsonFromFile(FString FilePath);

	static UWorld* GetWorld();
	static FString GetStudyFrameworkPath();
};