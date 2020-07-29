// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SFUtils.h"
#include "LogMacros.h"
#include "SFPlugin.h"


DEFINE_LOG_CATEGORY(SFLog);

void FSFUtils::OpenMessageBox(const FString Text, const bool bError)
{
	if (!FSFPlugin::GetIsMaster())
	{
		return;
	}
	
	FString TrueStatement;
	if (bError)
	{
		TrueStatement = "TRUE";
	}
	else
	{
		TrueStatement = "FALSE";
	}
	
	LogStuff("[FVAUtils::OpenMessageBox(ERROR = " + TrueStatement + ")]: Opening Message Box with message: " + Text);

	if (bError)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(AddExclamationMarkAroundChar(Text)));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Text));
	}

}


FString FSFUtils::AddExclamationMarkAroundChar(const FString Text)
{
	size_t Length = Text.Len();
	FString ReturnedString = FString("!!!!!");
	ReturnedString.Append(Text).Append("!!!!!");

	return ReturnedString;
}


void FSFUtils::LogStuff(const FString Text, const bool Error)
{
	if (Error)
	{
		UE_LOG(SFLog, Error, TEXT("%s"), *Text);
	}
	else
	{
		UE_LOG(SFLog, Log, TEXT("%s"), *Text);
	}
}

