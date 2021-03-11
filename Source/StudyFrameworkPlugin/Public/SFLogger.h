#pragma once


#include "CoreMinimal.h"
#include "JsonObject.h"
#include "SharedPointer.h"

class USFParticipant;
class USFGameInstance;
class USFStudyPhase;

#include "SFLogger.generated.h"

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFLogger : public UObject
{
    GENERATED_BODY()

public:
    USFLogger();

    UFUNCTION()
        void Initialize(USFParticipant* ParticipantNew, FString JsonFilePathNew, FString LogNameNew, FString SaveLogNameNew);

    // UFUNCTION()
    //     void InitializeJsonFile(TArray<USFStudyPhase*> Phases);


    void SaveJsonFile(TSharedPtr<FJsonObject> Json);        // TODO implement



    UFUNCTION()
        void LogData(FString String);

    UFUNCTION()
        void SaveData(FString NameOfData, FString Data, int PhaseIdx, FString Setup);       // TODO get rid of?

    UFUNCTION()
        void SaveDataArray(FString NameOfData, TArray<FString> Data, int PhaseIdx, FString Setup);

    UFUNCTION()
        void CommitData();          // TODO Make sure the path is only the path

    TSharedPtr<FJsonObject> GetJsonFile();

protected:

    TSharedPtr<FJsonObject> MainJsonObject; // TODO does it need to be initialized?

    UPROPERTY()
        USFParticipant* Participant;
        

    UPROPERTY()
        FString JsonFilePath;

    UPROPERTY()
        FString LogName = "DataLog";

    UPROPERTY()
        FString SaveLogName = "SaveLog";


    UPROPERTY()
        bool bInitialized = false;

    // UPROPERTY()
    //     USFGameInstance* GameInstance; // TODO actually needed?


};

