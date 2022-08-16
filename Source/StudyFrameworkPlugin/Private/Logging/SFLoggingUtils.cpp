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

void FSFLoggingUtils::SetupLoggingStreams()
{
	ILogStream* SFLog = UniLog.NewLogStream("SFLog", "StudyFramework/DebuggingLogs", "SFLog.txt", false);
	SFLog->SetLogToDefaultLog(true);

	//ParticipantLog, PositionLog are set up in Participant init function

	ILogStream* SFErrorLog = UniLog.NewLogStream("SFErrorLog", "StudyFramework/Logs", "SFLog.txt", false);
	SFErrorLog->SetLogToDefaultLog(true);
	SFErrorLog->SetPrefix(TEXT("Error: "));
	SFErrorLog->SetLogOnScreenOnMaster(true);
	SFErrorLog->SetOnScreenColor(FColor::Red);
}
