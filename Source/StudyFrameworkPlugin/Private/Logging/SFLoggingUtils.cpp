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
			LogToHUD("ERROR: "+Text);
		}
	}
	else
	{
		UniLog.Log(Text, "SFLog");
	}
}

void FSFLoggingUtils::SetupLoggingStreams()
{
	ILogStream* SFLog = UniLog.NewLogStream("SFLog", "StudyFramework/Logs", "SFLog.txt", false);
	SFLog->SetLogToDefaultLog(true);

	//ParticipantLog is set up in Participant init function

	//Used in the debugging process, overwritten for new session
	ILogStream* SFDebugLog = UniLog.NewLogStream("SFDebugLog", "StudyFramework/Logs", "SFDebugLog.txt", false);
	SFDebugLog->SetLogToDefaultLog(true);

	ILogStream* SFErrorLog = UniLog.NewLogStream("SFErrorLog", "StudyFramework/Logs", "SFLog.txt", false);
	SFErrorLog->SetLogToDefaultLog(true);
	SFErrorLog->SetPrefix(TEXT("Error: "));
	SFErrorLog->SetLogOnScreenOnMaster(true);
	SFErrorLog->SetOnScreenColor(FColor::Red);
}

void FSFLoggingUtils::LogData(const FString& DependentVariableName, const FString& Value)
{
	USFCondition* CurrCondition = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition();
	if (!CurrCondition->StoreDependentVariableData(DependentVariableName, Value))
	{
		FSFLoggingUtils::Log(
			"Cannot log data '" + Value + "' for dependent variable '" + DependentVariableName +
			"' since it does not exist for this condition!", true);
		return;
	}
	LogComment("Recorded " + DependentVariableName + ": " + Value);

	//the data is stored in the phase long table on SetCondition() or EndStudy()
}

void FSFLoggingUtils::LogComment(const FString& Comment, bool AlsoLogToHUD /*=false*/)
{
	UniLog.Log("#" + USFGameInstance::Get()->GetParticipant()->GetCurrentTime() + ": " + Comment, "ParticipantLog");
	FSFLoggingUtils::Log("Logged Comment: " + Comment);
	if(AlsoLogToHUD)
	{
		LogToHUD(Comment);
	}
}

void FSFLoggingUtils::LogToHUD(FString Text)
{
	if (FSFUtils::GetWorld()->GetFirstPlayerController() && Cast<ASFMasterHUD>(FSFUtils::GetWorld()->GetFirstPlayerController()->GetHUD()) &&
		Cast<ASFMasterHUD>(FSFUtils::GetWorld()->GetFirstPlayerController()->GetHUD())->IsWidgetPresent())
	{
		Cast<ASFMasterHUD>(FSFUtils::GetWorld()->GetFirstPlayerController()->GetHUD())->AddLogMessage(Text);
	}
	else
	{
		USFGameInstance::Get()->HUDSavedData.LogMessages.Add(Text);
	}
}
