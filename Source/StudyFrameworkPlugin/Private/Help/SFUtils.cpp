// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Help/SFUtils.h"

#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"

#include "EngineUtils.h"            // For Spawning in Actor in each level
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

	Log(FString("[FVAUtils::OpenMessageBox(ERROR = ") + (bError ? "TRUE" : "FALSE") +
	    ")]: Opening Message Box with message: " + Text, bError);

	FText Title = FText::FromString(FString(bError ? "ERROR" : "Message"));
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Text), &Title);
}


void FSFUtils::Log(const FString Text, const bool Error /*=false*/)
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

void FSFUtils::SetupLoggingStreams()
{
	ILogStream* SFLog = UniLog.NewLogStream("SFLog", "StudyFramework/Logs", "SFLog.txt", false);
	SFLog->SetLogToDefaultLog(true);

	ILogStream* SFErrorLog = UniLog.NewLogStream("SFErrorLog", "StudyFramework/Logs", "SFLog.txt", false);
	SFErrorLog->SetLogToDefaultLog(true);
	SFErrorLog->SetPrefix(TEXT("Error: "));
	SFErrorLog->SetLogOnScreenOnMaster(true);
	SFErrorLog->SetOnScreenColor(FColor::Red);
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

TSubclassOf<AActor> FSFUtils::GetBlueprintClass(FString BlueprintName, FString BlueprintPath)
{
	const FString FullName = BlueprintPath + "/" + BlueprintName + "." + BlueprintName;
	UBlueprint* BlueprintToSpawn = FindObject<UBlueprint>(ANY_PACKAGE, *FullName);

	if(!BlueprintToSpawn)
	{
		FSFUtils::Log("[FSFUtils::GetBlueprintClass)]: Cannot find blueprint actor ("
	              + BlueprintPath + "/" + BlueprintName + ")!", true);
		return nullptr;
	}

	UClass* Class = BlueprintToSpawn->GeneratedClass;
	
	if (!Class->IsChildOf(AActor::StaticClass()))
	{
		FSFUtils::Log("[FSFUtils::GetBlueprintClass)]: blueprint actor ("
		              + BlueprintPath + "/" + BlueprintName + ") is not a subclass of AActor!", true);
		return nullptr;
	}

	return Class;
}

