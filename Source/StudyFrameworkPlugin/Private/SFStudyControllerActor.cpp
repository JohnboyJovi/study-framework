// Fill out your copyright notice in the Description page of Project Settings.


#include "SFStudyControllerActor.h"


// #include <string>

#include "SFUtils.h"

// #include "Components/InputComponent.h"
// #include "Components/PointLightComponent.h"
#include "IUniversalLogging.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/World.h"


#include "SFGameInstance.h"
#include "SFPlugin.h"


ASFStudyControllerActor* ASFStudyControllerActor::Manager;

ASFStudyControllerActor::ASFStudyControllerActor()
{
	PrimaryActorTick.bCanEverTick = false;
}


void ASFStudyControllerActor::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<USFGameInstance>(GetGameInstance());
	if (GameInstance == nullptr)
	{
		FSFUtils::OpenMessageBox("GameInstance is not set to USFGameInstance, Study Framework will not work", true);
	}
}

void ASFStudyControllerActor::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}


bool ASFStudyControllerActor::StartStudy()
{
	if (GameInstance->IsStarted())
	{
		FSFUtils::Log("[ASFManagerActor::StartStudy()]: Study already started", false);
		return false;
	}

	GameInstance->StartStudy();

	return true;
}

bool ASFStudyControllerActor::NextCondition()
{
	if (!GameInstance->IsStarted())
	{
		FSFUtils::Log("[ASFManagerActor::NextCondition()]: Study not started yet", false);
		return false;
	}

	GameInstance->NextCondition();

	return true;
}


bool ASFStudyControllerActor::AddPhase(USFStudyPhase* Phase)
{
	if (GameInstance->IsStarted())
	{
		FSFUtils::Log("[ASFManagerActor::AddPhase()]: Study already started", false);
		return false;
	}

	GameInstance->AddPhase(Phase);

	return true;
}


bool ASFStudyControllerActor::AddActorForEachLevelCpp(UClass* Actor)
{
	if (GameInstance->IsStarted())
	{
		FSFUtils::Log("[ASFManagerActor::AddActorForEachLevel()]: Study already started", false);
		return false;
	}


	GameInstance->AddActorForEveryLevelInEveryPhaseCpp(Actor);

	return true;
}


bool ASFStudyControllerActor::AddActorForEachLevelBlueprint(FSFClassOfBlueprintActor Actor)
{
	if (GameInstance->IsStarted())
	{
		FSFUtils::Log("[ASFManagerActor::AddActorForEachLevel()]: Study already started", false);
		return false;
	}

	GameInstance->AddActorForEveryLevelInEveryPhaseBlueprint(Actor);

	return true;
}

void ASFStudyControllerActor::SetFadeColor(const FLinearColor Color)
{
	GameInstance->SetFadeColor(Color);
}

void ASFStudyControllerActor::SetFadeDuration(float FadeDuration)
{
	GameInstance->SetFadeDuration(FadeDuration);
}

void ASFStudyControllerActor::SetFadedOutDuration(float FadeOutWait)
{
	GameInstance->SetFadedOutDuration(FadeOutWait);
}

bool ASFStudyControllerActor::SetInitialFadedOut(bool bFadedOut)
{
	if (GameInstance->IsStarted())
	{
		FSFUtils::Log("[ASFManagerActor::SetInitialFadedOut()]: Study already started", false);
		return false;
	}

	GameInstance->SetInitialFadedOut(bFadedOut);

	return true;
}


ASFStudyControllerActor* ASFStudyControllerActor::GetCurrentControllerActor()
{
	return Manager;
}

void ASFStudyControllerActor::SaveDataArray(const FString DataName, TArray<FString> Data)
{
	GameInstance->SaveDataArray(DataName, Data);

	GameInstance->CommitData();
}

void ASFStudyControllerActor::SaveData(const FString DataName, FString Data)
{
	GameInstance->SaveData(DataName, Data);

	GameInstance->CommitData();
}

void ASFStudyControllerActor::LogData(const FString String)
{
	GameInstance->LogData(String);
}

void ASFStudyControllerActor::Initialize(FString ParticipantID, FString JsonFilePath)
{
	if (bInitialized)
	{
		return;
	}

	Manager = this;

	GameInstance = Cast<USFGameInstance>(GetGameInstance());

	if (!GameInstance)
	{
		FSFUtils::Log("[ASFManagerActor::BeginPlay()]: Could not get Game Instance correctly", true);
	}

	if (!GameInstance->IsInitialized())
	{
		GameInstance->Initialize(ParticipantID, JsonFilePath);
	}

	APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	GameInstance->Initialize(ParticipantID, JsonFilePath);

	bInitialized = true;
}
