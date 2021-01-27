#pragma once


#include "CoreMinimal.h"
#include "JsonObject.h"


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
        void Initialize(TArray<USFStudyPhase*> Phases);

    UFUNCTION()
        void InitializeJsonFile(TArray<USFStudyPhase*> Phases);
    
    UFUNCTION()
        void LogData(FString String);

    UFUNCTION()
        void SaveData(FString NameOfData, FString Data, int PhaseIdx, FString Setup);

    UFUNCTION()
        void CommitData();


protected:


    TSharedPtr<FJsonObject> MainJsonObject; // TODO does it need to be initialized?

    UPROPERTY()
        FString JsonFilePath;

    UPROPERTY()
        FString LogName = "DataLog";

    UPROPERTY()
        FString SaveDataLogName = "SaveDataLog";

    UPROPERTY()
        bool bInitialized = false;

    UPROPERTY()
        USFGameInstance* GameInstance;


};

