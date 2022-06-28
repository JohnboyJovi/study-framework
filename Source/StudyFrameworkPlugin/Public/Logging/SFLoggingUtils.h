// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"
#include "SFParticipant.h"

class FJsonObject;

class FSFLoggingUtils
{
public:

	// Log into Console and errors also on Screen, works also in Shipping build in contrast to UE_LOG
	static void Log(FString Text, bool Error = false);

	// To setup the debugging logs to be used
	static void SetupLoggingStreams();

	//use this to print something to the log panel of the HUD, e.g., to inform the experimenter
	UFUNCTION()
	static void LogToHUD(FString Text);
};

