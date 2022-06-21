// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Help/SFUtils.h"
#include "Logging/SFLoggingUtils.h"

#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"

#include "Json.h"

#include "SFPlugin.h"
#include "SFGameInstance.h"

#include "Utility/VirtualRealityUtilities.h"

#include "IUniversalLogging.h"

void FSFUtils::OpenMessageBox(const FString Text, const bool bError/*=false*/)
{
	if (!UVirtualRealityUtilities::IsMaster())
	{
		return;
	}

	FSFLoggingUtils::Log(FString("[FVAUtils::OpenMessageBox(ERROR = ") + (bError ? "TRUE" : "FALSE") +
	    ")]: Opening Message Box with message: " + Text, bError);

	FText Title = FText::FromString(FString(bError ? "ERROR" : "Message"));
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Text), &Title);
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

void FSFUtils::WriteJsonToFile(TSharedPtr<FJsonObject> Json, FString FilenName)
{
	FFileHelper::SaveStringToFile(JsonToString(Json), *(FPaths::ProjectDir() + "StudyFramework/" + FilenName));
}

TSharedPtr<FJsonObject> FSFUtils::ReadJsonFromFile(FString FilenName)
{
	FString JsonString;
	if(!FFileHelper::LoadFileToString(JsonString, *(FPaths::ProjectDir() + "StudyFramework/" + FilenName)))
	{
		return nullptr;
	}
	return StringToJson(JsonString);
}

USFGameInstance* FSFUtils::GetGameInstance()
{
	return Cast<USFGameInstance>(GEngine->GetWorld()->GetGameInstance());
}

UWorld* FSFUtils::GetWorld()
{
	return GEngine->GetWorld();
}