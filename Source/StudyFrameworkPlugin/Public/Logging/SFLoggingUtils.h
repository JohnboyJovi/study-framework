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
	static void SetupDebugLoggingStreams();

	static void SetupParticipantLoggingStream(const FString& ParticipantInfix);
};

