// Fill out your copyright notice in the Description page of Project Settings.


#include "SFStudyControllerActor.h"

// #include <string>

#include "SFUtils.h"

// #include "Components/InputComponent.h"
// #include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/World.h"


#include "SFGameInstance.h"


ASFStudyControllerActor* ASFStudyControllerActor::Manager;

ASFStudyControllerActor::ASFStudyControllerActor()
{
    PrimaryActorTick.bCanEverTick = false;
}


void ASFStudyControllerActor::BeginPlay()
{
    Super::BeginPlay();

    Initialize();

}

void ASFStudyControllerActor::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

}



bool ASFStudyControllerActor::StartStudy()
{
    if (!CheckGameInstance())
    {
        return false;
    }

    if (GameInstance->IsStarted())
    {
        FSFUtils::LogStuff("[ASFManagerActor::StartStudy()]: Study already started", false);
        return false;
    }

	GameInstance->StartStudy();

    return true;
}

bool ASFStudyControllerActor::NextSetup()
{
    if (!CheckGameInstance())
    {
        return false;
    }

    if (!GameInstance->IsStarted())
    {
        FSFUtils::LogStuff("[ASFManagerActor::NextSetup()]: Study not started yet", false);
        return false;
    }

	GameInstance->NextSetup();

    return true;
}

bool ASFStudyControllerActor::AddPhase(USFStudyPhase* Phase)
{
    if (!CheckGameInstance())
    {
        return false;
    }

    if (GameInstance->IsStarted())
    {
        FSFUtils::LogStuff("[ASFManagerActor::AddPhase()]: Study already started", false);
        return false;
    }

    GameInstance->AddPhase(Phase);

    return true;
}





bool ASFStudyControllerActor::AddActorForEachLevelCpp(UClass* Actor)
{
    if (!CheckGameInstance())
    {
        return false;
    }

    if (GameInstance->IsStarted())
    {
        FSFUtils::LogStuff("[ASFManagerActor::AddActorForEachLevel()]: Study already started", false);
        return false;
    }

    GameInstance->AddActorForEveryLevelInEveryPhaseCpp(Actor);

    return true;
}

bool ASFStudyControllerActor::CheckGameInstance()
{
    if (GameInstance == nullptr)
    {
        GameInstance = Cast<USFGameInstance>(GetGameInstance());
        if (GameInstance == nullptr)
        {
            return false;
        }
    }
    return true;
}

bool ASFStudyControllerActor::AddActorForEachLevelBlueprint(FSFClassOfBlueprintActor Actor)
{
    if(!CheckGameInstance())
    {
        return false;
    }

    if (GameInstance->IsStarted())
    {
        FSFUtils::LogStuff("[ASFManagerActor::AddActorForEachLevel()]: Study already started", false);
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
    if (!CheckGameInstance())
    {
        return false;
    }

    if (GameInstance->IsStarted())
    {
        FSFUtils::LogStuff("[ASFManagerActor::SetInitialFadedOut()]: Study already started", false);
        return false;
    }

    GameInstance->SetInitialFadedOut(bFadedOut);

    return true;
}



ASFStudyControllerActor* ASFStudyControllerActor::GetCurrentControllerActor()
{
	return Manager;
}

void ASFStudyControllerActor::SaveData(const FString DataName, FString Data)
{
    GameInstance->SaveData(DataName, Data);
}

void ASFStudyControllerActor::LogData(const FString String)
{
    GameInstance->LogData(String);
}

void ASFStudyControllerActor::CommitData()
{
    GameInstance->CommitData();
}

void ASFStudyControllerActor::Initialize()
{
    if (bInitialized)
    {
        return;
    }

    Manager = this;

    GameInstance = Cast<USFGameInstance>(GetGameInstance());

    if (!GameInstance)
    {
        FSFUtils::LogStuff("[ASFManagerActor::BeginPlay()]: Could not get Game Instance correctly", true);
    }

    if (!GameInstance->IsInitialized())
    {
        GameInstance->Initialize();
    }

    APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    GameInstance->Initialize();

    bInitialized = true;
}
