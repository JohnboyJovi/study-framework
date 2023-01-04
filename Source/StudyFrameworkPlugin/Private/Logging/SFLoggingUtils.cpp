// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Logging/SFLoggingUtils.h"

#include "Json.h"

#include "SFGameInstance.h"

#include "Utility/VirtualRealityUtilities.h"

#include "IUniversalLogging.h"
#include "Help/SFUtils.h"


void FSFLoggingUtils::Log(const FString Text, const bool Error /*=false*/)
{
	if (Error)
	{
		UniLog.Log(Text, "SFErrorLog");
		if(USFGameInstance::IsInitialized())
		{
			//to avoid endless error message loops
			USFGameInstance::Get()->LogToHUD("ERROR: "+Text);
		}
	}
	else
	{
		UniLog.Log(Text, "SFLog");
	}
}

void FSFLoggingUtils::SetupDebugLoggingStreams()
{

	const FString Timestamp = FDateTime::Now().ToString();
	FString LogFilename = "SFLog_" + Timestamp + ".txt";

	ILogStream* SFLog = UniLog.NewLogStream("SFLog", "StudyFramework/DebuggingLogs", LogFilename, false);
	SFLog->SetLogToDefaultLog(true);

	//ParticipantLog, PositionLog, GazeTrackingLog are set up in SFParticipant::SetupDebugLoggingStreams() function

	ILogStream* SFErrorLog = UniLog.NewLogStream("SFErrorLog", "StudyFramework/DebuggingLogs", LogFilename, false);
	SFErrorLog->SetLogToDefaultLog(true);
	SFErrorLog->SetPrefix(TEXT("Error: "));
	SFErrorLog->SetLogOnScreenOnMaster(true);
	SFErrorLog->SetOnScreenColor(FColor::Red);
}

void FSFLoggingUtils::SetupParticipantLoggingStream(const FString& ParticipantInfix)
{
	ILogStream* ParticipantLog = UniLog.NewLogStream("ParticipantLog", "StudyFramework/StudyLogs/ParticipantLogs",
		ParticipantInfix + ".txt", false);
}
