// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"
#include "SFParticipant.h"

class FJsonObject;

class FSFUtils
{
public:

	// Open Message Box with Text. if error is set, the window is titled ERROR //
	static void OpenMessageBox(FString Text, bool bError = false);

	static FString JsonToString(TSharedPtr<FJsonObject> Json);
	static TSharedPtr<FJsonObject> StringToJson(FString String);

	static void WriteJsonToFile(TSharedPtr<FJsonObject> Json, FString FilenName);
	static TSharedPtr<FJsonObject> ReadJsonFromFile(FString FilenName);

	static UWorld* GetWorld();
};