// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SFDefinesPrivate.h"

#include "SFParticipant.generated.h"



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

	void Initialize(FString IdN, TArray<FStudySettingU> ConfigurationsN);

	// TODO Safe Configuration as it is
	bool SaveConfigurationFile();

	// TODO Search for Configuration
	bool FindConfigurationFile();

	// TODO Load Configuration from files
	bool LoadConfigurationFile();

	// TODO 
	bool CreateNewConfigurationFile(TArray<FStudySettingU> ConfigurationsN);

	// TODO 
	bool DeleteConfigurationFile();

	// TODO Set up the Array SetupOrder
	bool CreateSetupOrder();

	// TODO Log the Setup Order
	bool LogSetupOrder();
	
	// Function to get the next planned setup
	TArray<int> GetNextSetup();
	

protected:
	FString Id;
	bool* AlreadyDone;
	TArray<FStudySettingU> Configurations;
	int TotalNumberOfConfigurations = 1;

	TArray<TArray<int>> SetupOrder;
	int NextSetup = 0;
};
