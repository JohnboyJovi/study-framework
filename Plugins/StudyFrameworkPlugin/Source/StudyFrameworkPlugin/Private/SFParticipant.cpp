// Fill out your copyright notice in the Description page of Project Settings.


#include "SFParticipant.h"

#include <string>



#include "Engine/Engine.h"
#include "SFUtils.h"

USFParticipant::USFParticipant()
{	
}

USFParticipant::~USFParticipant()
{
}

void USFParticipant::Initialize(FString IdN, TArray<FStudySettingU> ConfigurationsN)
{
	Id = IdN;
	if (FindConfigurationFile())
	{
		LoadConfigurationFile();
	}
	else
	{
		CreateNewConfigurationFile(ConfigurationsN);
	}

	CreateSetupOrder();

	LogSetupOrder();

}

bool USFParticipant::SaveConfigurationFile()
{
	return false;
}

bool USFParticipant::FindConfigurationFile()
{
	return false;
}

bool USFParticipant::LoadConfigurationFile()
{
	return false;
}

bool USFParticipant::CreateNewConfigurationFile(TArray<FStudySettingU> ConfigurationsN)
{
	for (int i = 0; i < ConfigurationsN.Num(); i++)
	{
		TotalNumberOfConfigurations = TotalNumberOfConfigurations * ConfigurationsN[i].Count;
		FStudySettingU Tmp;
		Tmp.Count = ConfigurationsN[i].Count;
		Tmp.Name = ConfigurationsN[i].Name;
		Configurations.Add(Tmp);
	}

	SetupOrder.Empty();
	SetupOrder.SetNum(TotalNumberOfConfigurations);

	// TODO Maybe TArray? 
	AlreadyDone = new bool[TotalNumberOfConfigurations];

	for (int i = 0; i < TotalNumberOfConfigurations; i++)
	{
		AlreadyDone[i] = false;
	}

	SaveConfigurationFile();

	return false;
}

bool USFParticipant::DeleteConfigurationFile()
{
	return false;
}

bool USFParticipant::CreateSetupOrder()
{
	int NumberOfConf = Configurations.Num();

	// Setting up a counter to be able to select every single combination 
	TArray<int> Counter;
	Counter.SetNum(NumberOfConf, false);

	for (int i = 0; i < NumberOfConf; i++)
	{
		Counter[i] = 1;
	}

	
	for (int i = 0; i < TotalNumberOfConfigurations; i++)
	{
		SetupOrder[i] = Counter;
		
		for (int j = 0; j < NumberOfConf; j++)
		{
			if(Counter[j] < Configurations[j].Count)
			{
				Counter[j] = Counter[j] + 1;;
				break;
			}

			Counter[j] = 1;
		}
	}
	
	return false;
}

bool USFParticipant::LogSetupOrder()
{
	FSFUtils::LogStuff("[SFParticipant::LogSetupOrder()]: Logging Setup Order now..");
	for (int i = 0; i < SetupOrder.Num(); i++)
	{
		FString Text = "";
		for (int j = 0; j < Configurations.Num(); j++)
		{
			Text = Text + FString::FromInt(SetupOrder[i][j]) + "  ";
		}
		FSFUtils::LogStuff(Text, false);

		// Also print it on Screen
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-10, 1.f, FColor::Yellow, Text);
		}
	}

	return true;
}

TArray<int> USFParticipant::GetNextSetup()
{
	if (NextSetup >= TotalNumberOfConfigurations)
	{
		FSFUtils::LogStuff("[SFParticipant::GetNextSetup()]: There is no next setup", true);
		return TArray<int>();
	}

	return SetupOrder[NextSetup++];
	
}
