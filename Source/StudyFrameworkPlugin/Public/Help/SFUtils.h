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
	static int OpenCustomDialog(const FString& Title, const FString& Content, const TArray<FString>& Buttons);
	static int OpenCustomDialogText(const FString& Title, const FString& Content, const FString& DefaultText, FString& OutText);


	static FString JsonToString(TSharedPtr<FJsonObject> Json);
	static TSharedPtr<FJsonObject> StringToJson(FString String);

	static void WriteJsonToFile(TSharedPtr<FJsonObject> Json, FString FilePath);
	static TSharedPtr<FJsonObject> ReadJsonFromFile(FString FilePath);

	static UWorld* GetWorld();
	static FString GetStudyFrameworkPath();
};