#include "Help/SFLogger.h"
#include "Help/SFUtils.h"
#include "SFGameInstance.h"

#include "IUniversalLogging.h"
#include "Json.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Utility/VirtualRealityUtilities.h"
// #include "FileHelper.h"

USFLogger::USFLogger()
{
	MainJsonObject = MakeShared<FJsonObject>();

	return;
}


void USFLogger::Initialize(USFParticipant* ParticipantNew, FString JsonFilePathNew, FString LogNameNew,
                           FString SaveLogNameNew)
{
	Participant = ParticipantNew;
	JsonFilePath = JsonFilePathNew;
	LogName = LogNameNew;
	SaveLogName = SaveLogNameNew;

	//ILogStream* LogStream = UniLog.NewLogStream(LogName, "Saved/OwnLogs/", LogName + ".log", true);
	//TODO what is the difference here?
	//LogStream = UniLog.NewLogStream(SaveLogName, "Saved/OwnLogs/", SaveLogName + ".log", true); // TODO Kommentar

	bInitialized = true;
}


void USFLogger::SaveJsonFile(TSharedPtr<FJsonObject> Json)
{
}

void USFLogger::LogData(FString String)
{
	if (!bInitialized)
	{
		FSFUtils::Log("[USFLogger::LogData()]: Not initialized yet", false);
		return;
	}

	UniLog.Log(String, LogName);
}


void USFLogger::SaveData(const FString NameOfData, FString Data, const int PhaseIdx, const FString Setup)
{
	TArray<FString> DataArray;

	DataArray.Add(Data);

	SaveDataArray(NameOfData, DataArray, PhaseIdx, Setup);
}

void USFLogger::SaveDataArray(FString NameOfData, TArray<FString> Data, int PhaseIdx, FString Setup)
{
	if (!bInitialized)
	{
		FSFUtils::Log("[USFLogger::SaveData()]: Not initialized yet", false);
		return;
	}

	/*
	 + Phases (object)
	 +     Num (Int)
	 +         5 (Value)
	 +     1 (object)
	 +         Setup (Array)
	 +             2_2_2 (Value)
	 +         Order (FString Array)
	 +             1_1,1_2,2_1,2_2 (Value)

	 + "Data": (object)
	 +     "Phase_1": (object)
	 +         "1_1": (object)
	 -             "NameOfData": (Array)
	 >                  1,2,3,4,5 (Value)
	*/

	TSharedPtr<FJsonObject> Json = MainJsonObject->GetObjectField("Data")
	                                             ->GetObjectField(FString::FromInt(PhaseIdx))
	                                             ->GetObjectField(Setup);

	TArray<TSharedPtr<FJsonValue>> Array;
	TArray<FString> ArrayFString;

	if (Json->HasField(NameOfData))
	{
		Json->TryGetStringArrayField(NameOfData, ArrayFString);
	}

	ArrayFString.Append(Data);

	for (auto Entry : ArrayFString)
	{
		TSharedPtr<FJsonValueString> DataJson = MakeShared<FJsonValueString>(Entry);
		Array.Add(DataJson);
	}

	Json->SetArrayField(NameOfData, Array);

	FString DataString = Data[0];
	for (int i = 1; i < Data.Num(); i++)
	{
		DataString = DataString + ", " + Data[i];
	}


	LogData("[SaveData]: {" + NameOfData + "}, [Data]: {" + DataString + "}");
}

void USFLogger::CommitData()
{
	if (!bInitialized)
	{
		FSFUtils::Log("[USFLogger::CommitData()]: Not initialized yet", false);
		return;
	}

	if(!UVirtualRealityUtilities::IsMaster())
	{
		return;
	}

	FString SavedFolder = FPaths::ProjectSavedDir();

	FString JsonFile = SavedFolder + JsonFilePath + Participant->GetID() + ".txt";
	FString JsonFileTmp = SavedFolder + JsonFilePath + Participant->GetID() + "_tmp.txt";

	bool Success = true;

	// Get String from Json
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(MainJsonObject.ToSharedRef(), Writer);

	// Log it in a extra log
	//UniLog.Log(JsonString, SaveLogName);

	// Write to File_Tmp
	if (!FFileHelper::SaveStringToFile(JsonString, *JsonFileTmp))
	{
		Success = false;
	}

	// Make Tmp file the actual file
	if (IFileManager::Get().FileExists(*JsonFile))
	{
		if (!IFileManager::Get().Delete(*JsonFile))
		{
			Success = false;
			FSFUtils::Log("[USFLogger::CommitData()]: Could not delete old json", true);
		}
	}
	else
	{
		FSFUtils::Log("[USFLogger::CommitData()]: Could not find old json " + JsonFile, false);
	}

	if (!IFileManager::Get().Move(*JsonFile, *JsonFileTmp))
	{
		Success = false;
		FSFUtils::Log("[USFLogger::CommitData()]: Could not rename new json " + JsonFileTmp, true);
	}
}

TSharedPtr<FJsonObject> USFLogger::GetJsonFile()
{
	return MainJsonObject;
}
