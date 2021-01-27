// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SFDefinesPublic.h"

#include "SFParticipant.generated.h"

class USFGameInstance;

/**
 * 
 */
// TODO UCLASS for saving the whole state of the Participant
UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFParticipant : public UObject
{
	GENERATED_BODY()
	
public:
	USFParticipant();
	~USFParticipant();

	void Initialize(FString IdN, USFGameInstance* GameInstance);

	// TODO Safe Configuration as it is
	bool SaveConfigurationFile();

	// TODO Search for Configuration
	bool FindConfigurationFile();

	// TODO Load Configuration from files
	bool LoadConfigurationFile();

	// TODO implement
	bool CreateNewConfigurationFile();

	// TODO implement
	bool DeleteConfigurationFile();

	// TODO Set up the Array SetupOrder
	bool CreateSetupOrder();

	// TODO Log the Setup Order
	bool LogSetupOrder();
	
	// Function to get the next planned setup
	TArray<int> GetNextSetup();
	

protected:
	FString Id;
	

	USFGameInstance* GameInstance;
	
	// TArray<FSFStudySetting> Settings;
	// TArray<TArray<int>> SetupOrder;
	// TArray<bool> AlreadyDone;
	int NextSetupIdx = 0;

	TMap<int, int> PhaseTotalNumberOfConfigurations;

    TMap<int, TArray<FSFStudySetting>> PhaseSettings;
    TMap<int, TArray<TArray<int>>> PhaseSetupOrder;
    TMap<int, TArray<bool>> PhaseAlreadyDone;
    TArray<int> Phases;

    int CurrentPhaseIdx = 0;
    int CurrentPhase;

    // If Study is finished
    bool bFinished = false;





public:
	// USFParticipant* operator=(const USFParticipant* Other) // copy assignment
	// {	
	// 	Id								= Other->Id;
	// 	TotalNumberOfConfigurations		= Other->TotalNumberOfConfigurations;
	// 	AlreadyDone						= new bool[TotalNumberOfConfigurations];
	// 
	// 	for (int i = 0; i < TotalNumberOfConfigurations; i++)
	// 	{
	// 		AlreadyDone[i] = Other->AlreadyDone[i];
	// 	}
	// 
	// 	Configurations					= Other->Configurations;
	// 	SetupOrder						= Other->SetupOrder;
	// 	NextSetup						= Other->NextSetup;
	// 	
	// 	return this;
	// }
};
