

#include "SFLogger.h"
#include "SFUtils.h"
#include "SFGameInstance.h"

#include "IUniversalLogging.h"
#include "Json.h"

#include "FileManager.h"
// #include "FileHelper.h"

USFLogger::USFLogger()
{
    return;
}

void USFLogger::Initialize(TArray<USFStudyPhase*> Phases)
{
    UniLog.NewLogStream(LogName, "Saved/OwnLogs/", LogName + ".log", true);
    UniLog.NewLogStream(SaveDataLogName, "Saved/OwnLogs/", SaveDataLogName + ".log", true);
    InitializeJsonFile(Phases);

    bInitialized = true;
}

void USFLogger::InitializeJsonFile(TArray<USFStudyPhase*> Phases)
{
    MainJsonObject = MakeShared<FJsonObject>();

    TSharedPtr<FJsonObject> JsonPhases  = MakeShared<FJsonObject>();
    TSharedPtr<FJsonObject> JsonData    = MakeShared<FJsonObject>();
    
    const int NumPhases = Phases.Num();

    JsonPhases->SetNumberField("NumberOfPhases", NumPhases);

    TArray<TSharedPtr<FJsonObject>> JsonEachPhase;

    for (int i = 0; i < NumPhases; i++)
    {
        TSharedPtr<FJsonObject> JsonTmpPhases   = MakeShared<FJsonObject>();
        TSharedPtr<FJsonObject> JsonTmpData     = MakeShared<FJsonObject>();

        // Setup
        TArray<int> SetupInt = Phases[i]->GetSetupNumArray();
        TArray<TSharedPtr<FJsonValue>> Setup;
        for (auto Entry : SetupInt)
        {
            TSharedPtr<FJsonValueNumber> Tmp = MakeShared<FJsonValueNumber>(Entry);
            Setup.Add(Tmp);
        }
        JsonTmpPhases->SetArrayField("Setup", Setup);

        // Order
        TArray<FString> OrderFString = Phases[i]->GetSetupOrderArrayString();
        TArray<TSharedPtr<FJsonValue>> Order;
        for (auto Entry : OrderFString)
        {
            TSharedPtr<FJsonValueString> Tmp = MakeShared<FJsonValueString>(Entry);
            Order.Add(Tmp);
        }
        JsonTmpPhases->SetArrayField("Order", Setup);

        // Add Phase
        JsonPhases->SetObjectField(FString::FromInt(i), JsonTmpPhases);


        // Data Part
        for (auto Entry : OrderFString)
        {
            TSharedPtr<FJsonObject> Tmp = MakeShared<FJsonObject>();
            JsonTmpData->SetObjectField(Entry, Tmp);
        }

        JsonData->SetObjectField(FString::FromInt(i), JsonTmpData);

    }

    MainJsonObject->SetObjectField("Phases", JsonPhases);
    MainJsonObject->SetObjectField("Data", JsonData);
}


void USFLogger::LogData(FString String)
{
    if (!bInitialized)
    {
        FSFUtils::LogStuff("[USFLogger::LogData()]: Not initialized yet", false);
        return;
    }

    UniLog.Log(String, LogName);
}


void USFLogger::SaveData(const FString NameOfData, FString Data, const int PhaseIdx, const FString Setup) 
{
    if (!bInitialized)
    {
        FSFUtils::LogStuff("[USFLogger::SaveData()]: Not initialized yet", false);
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

    ArrayFString.Add(Data);

    for (auto Entry : ArrayFString)
    {
        TSharedPtr<FJsonValueString> DataJson = MakeShared<FJsonValueString>(Data);
        Array.Add(DataJson);
    }

    Json->SetArrayField(NameOfData, Array);

    LogData("[SaveData]: {" + NameOfData + "}, [Data]: {" + Data + "}");
}

void USFLogger::CommitData()
{
    if (!bInitialized)
    {
        FSFUtils::LogStuff("[USFLogger::CommitData()]: Not initialized yet", false);
        return;
    }

    bool Success = true;

    // Get String from Json
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(MainJsonObject.ToSharedRef(), Writer);

    // Log it in a extra log
    UniLog.Log(JsonString, SaveDataLogName);

    // Write to File_Tmp
    const FString JsonFilePathTmp = JsonFilePath + "_Tmp";
    if (!FFileHelper::SaveStringToFile(JsonString, *JsonFilePathTmp))
    {
        Success = false;
    }

    // Make Tmp file the actual file
    if(IFileManager::Get().FileExists(*JsonFilePath))
    {
        if (!IFileManager::Get().Delete(*JsonFilePath))
        {
            Success = false;
            FSFUtils::LogStuff("[USFLogger::CommitData()]: Could not delete old json", true);
        }
    }
    else
    {
        FSFUtils::LogStuff("[USFLogger::CommitData()]: Could not find old json " + JsonFilePath, false);
    }

    if (!IFileManager::Get().Move(*JsonFilePathTmp, *JsonFilePath))
    {
        Success = false;
        FSFUtils::LogStuff("[USFLogger::CommitData()]: Could not rename new json " + JsonFilePathTmp, true);
    }

}



