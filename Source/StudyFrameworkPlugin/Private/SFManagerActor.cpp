// Fill out your copyright notice in the Description page of Project Settings.


#include "SFManagerActor.h"
#include "SFUtils.h"

// Sets default values
ASFManagerActor::ASFManagerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASFManagerActor::BeginPlay()
{
	Super::BeginPlay();

	TArray<FStudySetting> Configs;

	FSFUtils::OpenMessageBox("At ASFManagerActor::BeginPlay()", false);

	// TODO: Setup Real Test Scenario
	FStudySetting S1;
	S1.Name = "Setting 1 (Map)";
	S1.Count = 3;
	Configs.Add(S1);

	FStudySetting S2;
	S2.Name = "Setting 2 (Distance)";
	S2.Count = 2;
	Configs.Add(S2);


	FStudySetting S3;
	S3.Name = "Setting 3 (Brightness)";
	S3.Count = 2;
	Configs.Add(S3);

	TArray<FStudySettingU> ConfigsU;
	
	for (int i = 0; i < Configs.Num(); i++)
	{
		FStudySettingU Tmp = Configs[i];
		ConfigsU.Add(Tmp);
	}

	CurrentParticipant = NewObject<USFParticipant>();
	CurrentParticipant->Initialize("Test", ConfigsU);
	// CurrentParticipant = new USFParticipant("Test", ConfigsU);

	int a = 1;
	
}

// Called every frame
void ASFManagerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ASFManagerActor::StartStudy()
{
	if (bStudyStarted)
	{
		FSFUtils::LogStuff("[ASFManagerActor::StartStudy()]: Study already started.", true);
	}

	bStudyStarted = true;
	
	// TODO Let the Study start..
	
}

void ASFManagerActor::NextSetup()
{
	
}

void ASFManagerActor::AddStudySetting(FStudySetting Setting)
{
	if (bStudyStarted)
	{
		FSFUtils::LogStuff("[ASFManagerActor::AddStudySetting()]: Study already started.", true);
	}
	Settings.Add(Setting);
}

