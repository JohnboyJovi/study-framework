// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SFUtils.h"

#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"

#include "EngineUtils.h"            // For Spawning in Actor in each level
#include "Json.h"

#include "SFPlugin.h"

#include "IUniversalLogging.h"

void FSFUtils::OpenMessageBox(const FString Text, const bool bError/*=false*/)
{
	if (!FSFPlugin::GetIsMaster())
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
	}
	else
	{
		UniLog.Log(Text, "SFLog");
	}
}

void FSFUtils::SetupLoggingStreams()
{
	ILogStream* SFLog = UniLog.NewLogStream("SFLog", "Saved/Logs", "SFLog.txt", false);
	SFLog->SetLogToDefaultLog(true);

	ILogStream* SFErrorLog = UniLog.NewLogStream("SFErrorLog", "Saved/Logs", "SFLog.txt", false);
	SFErrorLog->SetLogToDefaultLog(true);
	SFErrorLog->SetPrefix(TEXT("Error: "));
	SFErrorLog->SetLogOnScreenOnMaster(true);
	SFErrorLog->SetOnScreenColor(FColor::Red);
}

FString FSFUtils::ConditionToString(TArray<FString> Condition)
{
	FString Output = Condition[0];
	for (int i = 1; i < Condition.Num(); i++)
	{
		Output = Output + "_" + Condition[i];
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

void FSFUtils::WriteJsonToFile(TSharedPtr<FJsonObject> Json, FString FilenName)
{
	FFileHelper::SaveStringToFile(JsonToString(Json), *(FPaths::ProjectSavedDir() + FilenName));
}

TSubclassOf<AActor> FSFUtils::GetBlueprintClass(FString BlueprintName, FString BlueprintPath)
{
	BlueprintName.Append(FString("_C"));

	TArray<UObject*> TmpArray;

	if (EngineUtils::FindOrLoadAssetsByPath(*BlueprintPath, TmpArray, EngineUtils::ATL_Class))
	{
		for (int i = 0; i < TmpArray.Num(); ++i)
		{
			UObject* Tmp = TmpArray[i];
			if (Tmp == nullptr || (!dynamic_cast<UClass*>(Tmp)) || (Tmp->GetName().Compare(BlueprintName) != 0))
			{
				continue;
			}

			if (Cast<AActor>(Tmp) == nullptr)
			{
				FSFUtils::Log("[FSFUtils::GetBlueprintClass)]: blueprint actor ("
				              + BlueprintPath + "/" + BlueprintName + ") is not a subclass of AActor!", true);
				return nullptr;
			}

			return Tmp->GetClass();
		}
	}
	FSFUtils::Log("[FSFUtils::GetBlueprintClass)]: Cannot find blueprint actor ("
	              + BlueprintPath + "/" + BlueprintName + ")!", true);
	return nullptr;
}

/*
 */
