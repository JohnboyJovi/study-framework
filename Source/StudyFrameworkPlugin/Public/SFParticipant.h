// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "JsonObject.h"
#include "SFDefinesPublic.h"
#include "SFStudyPhase.h"


#include "SFParticipant.generated.h"

class USFLogger;
class USFGameInstance;


UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFParticipant : public UObject
{
	GENERATED_BODY()
	
public:
	USFParticipant();
	~USFParticipant();

	bool Initialize(FString IdNew, FString JsonFilePath, USFGameInstance* GameInstanceNew, 
        FString LogName = "NormalLog", FString SaveDataLogName = "SaveLog");

    void GenerateInitialJsonFile(); // TODO implement GenerateInitialJsonFile()

    bool FindJsonFile();       // TODO implement Participant::FindJsonFile()
    bool LoadJsonFile();       // TODO implement Participant::LoadJsonFile()

    bool StartStudy();
    FString NextCondition();
    void EndStudy();            // TODO implement Participant::EndStudy()

    void AddPhase(USFStudyPhase* PhaseNew);
    bool CheckPhases();


    void SaveDataArray(FString Where, TArray<FString> Data);
    void LogData(FString Data);
    void CommitData();              // TODO need CommitData()?


    TSharedPtr<FJsonObject> GetJsonFile();
    USFStudyPhase* GetCurrentPhase();
    int GetCurrentPhaseIdx();
    FString GetID();


protected:
    UPROPERTY()
        FString ParticipantID;


    TSharedPtr<FJsonObject> MainJsonObject;


    UPROPERTY()
	    USFGameInstance* GameInstance; 

    UPROPERTY()
        USFLogger* Logger;


    UPROPERTY()
        TArray<int> UpcomingCondition;

    UPROPERTY()
        TArray<USFStudyPhase*> Phases;
    UPROPERTY()
        USFStudyPhase* CurrentPhase;
    UPROPERTY()
        int CurrentPhaseIdx;


};
