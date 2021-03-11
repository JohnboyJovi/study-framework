// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SFUtils.h"

#include "Engine/Engine.h"
#include "LogMacros.h"
#include "SFPlugin.h"

#include "Json.h"


int FSFUtils::KeyCounter = -1;

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
	
	LogStuff("[FVAUtils::OpenMessageBox(ERROR = " + TrueStatement + ")]: Opening Message Box with message: " + Text, bError);

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
		PrintToScreen("[Error]: " + Text);
	}
	else
	{
		UE_LOG(SFLog, Log, TEXT("%s"), *Text);
        PrintToScreen(Text);
	}
}

void FSFUtils::PrintToScreen(const FString Text, const float Time, const FColor Color)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(KeyCounter--, Time, Color, Text);
	}
}

FString FSFUtils::SetupToString(TArray<int> Setup)
{
    if (Setup.Num() == 0)
    {
        return "";
    }

    FString Output = FString::FromInt(Setup[0]);
    for (int i = 1; i < Setup.Num(); i++)
    {
        Output = Output + "_" + FString::FromInt(Setup[i]);
    }

    return Output;
}

FString FSFUtils::JsonToString(TSharedPtr<FJsonObject> Json)
{
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

    return OutputString;
}

TSharedPtr<FJsonObject> FSFUtils::StringToJson(FString String)
{
    TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
    TSharedRef<TJsonReader<TCHAR>> Reader = FJsonStringReader::Create(String);
    FJsonSerializer::Deserialize(Reader, Json);

    return Json;
}
/*
 */