// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"

class FJsonObject;

class FSFUtils
{
public:

	// Open Message Box with Text. if error is set, the window is titled ERROR //
	static void OpenMessageBox(FString Text, bool bError = false);

	// Log into Console and errors also on Screen, works also in Shipping build in contrast to UE_LOG
	static void Log(FString Text, bool Error = false);

	// To setup the debugging logs to be used
	static void SetupLoggingStreams();

	static FString JsonToString(TSharedPtr<FJsonObject> Json);
	static TSharedPtr<FJsonObject> StringToJson(FString String);

	static void WriteJsonToFile(TSharedPtr<FJsonObject> Json, FString FilenName);
	static TSharedPtr<FJsonObject> ReadJsonFromFile(FString FilenName);
};

