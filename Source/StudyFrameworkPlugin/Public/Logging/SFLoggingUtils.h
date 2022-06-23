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

	//Log Data collected for a DependentVariable in this condition
	UFUNCTION(BlueprintCallable)
	static void LogData(const FString& DependentVariableName, const FString& Value);

	//Log a comment (with a timestamp) to store events or user action etc. in the participant's log file
	//and optionally log to HUD (default=false)
	UFUNCTION(BlueprintCallable)
	static void LogComment(const FString& Comment, bool AlsoLogToHUD=false);

	//use this to print something to the log panel of the HUD, e.g., to inform the experimenter
	UFUNCTION()
	static void LogToHUD(FString Text);
};

