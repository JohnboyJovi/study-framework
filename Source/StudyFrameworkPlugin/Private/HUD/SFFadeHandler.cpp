// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/SFFadeHandler.h"


#include "EditorWidgets/Public/EditorWidgets.h"
#include "Kismet/GameplayStatics.h"
#include "SFGameInstance.h"

#include "SFUtils.h"

#include "TimerManager.h"						// For Tick Timer
#include "HUD/SFGlobalFadeGameViewportClient.h"		// For Fade


void USFFadeHandler::Tick()
{
	if (FadeState == EFadeState::NotFading || (FadeTimeRemaining() > 0.0f && FadeState != EFadeState::WaitForLevelLoaded)
	)
	{
		return;
	}

	switch (FadeState)
	{
		// Its Faded out, Load new Level now
	case EFadeState::FadingOut:
		FSFUtils::Log("[USFFadeHandler::Tick()]: Opening Level now", false);
		UGameplayStatics::OpenLevel(GameInstance->GetWorld(), *NewLevelName, false);
		SetTimerForNextTick();
		FadeState = EFadeState::WaitForLevelLoaded;
		break;

		// Its Faded out, New Level is loaded, prepare it
	case EFadeState::WaitForLevelLoaded:
		FSFUtils::Log("[USFFadeHandler::Tick()]: Set Scene to black on new Level (pre) loaded", false);
		GameInstance->SpawnAllActorsForLevel();
		Fade(0.0, true);
		FadeState = EFadeState::WaitForTimerFadedOut;
		SetTimerForNextTick(FadeOutWait);
		break;

		// Its Faded out, Timer for Fading in is done
	case EFadeState::WaitForTimerFadedOut:
		GameInstance->OnLevelLoaded();
		FSFUtils::Log("[USFFadeHandler::Tick()]: Fading in now", false);
		FadeIn();
		break;

		// Its Faded in, everything done, cleanup
	case EFadeState::FadingIn:
		FSFUtils::Log("[USFFadeHandler::Tick()]: Cleaning up", false);
		GameInstance->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		GameInstance->OnFadedIn();
		FadeState = EFadeState::NotFading;
		break;

	case EFadeState::NotFading:
	default:
		FSFUtils::Log("[USFFadeHandler::Tick()]: in Default or Not Fading switch case", true);
		return;
	}
}

void USFFadeHandler::FadeToLevel(const FString LevelName, const bool bStartFadedOut)
{
	if (GetCameraManager() == nullptr)
	{
		return;
	}

	if (GetIsFading())
	{
		return;
	}

	FSFUtils::Log(
		"[USFFadeHandler::FadeToLevel()]: Fading From level (" + GameInstance->GetWorld()->GetMapName() + ") to level (" +
		LevelName + ")", false);
	if (bStartFadedOut || bIsFadedOut)
	{
		if (bIsFadedOut)
		{
			bIsFadedOut = false;
		}
		Fade(0.0f, true);
		FadeState = EFadeState::FadingOut;
	}
	else
	{
		Fade(FadeDuration, true);
		FadeState = EFadeState::FadingOut;
	}


	NewLevelName = LevelName;
	SetTimerForNextTick();
}


void USFFadeHandler::FadeIn()
{
	if (!GetCameraManager())
	{
		return;
	}
	Fade(FadeDuration, false);
	FadeState = EFadeState::FadingIn;
	SetTimerForNextTick();
}


float USFFadeHandler::FadeTimeRemaining() const
{
	const UWorld* World = GameInstance->GetWorld();
	if (World)
	{
		USFGlobalFadeGameViewportClient* GameViewportClient = Cast<USFGlobalFadeGameViewportClient>(
			World->GetGameViewport());
		if (GameViewportClient)
		{
			return GameViewportClient->FadeTimeRemaining();
		}
	}

	return 0.0f;
}


void USFFadeHandler::Fade(const float Duration, const bool bToBlack) const
{
	if (Duration >= 0.0f)
	{
		if (bToBlack)
		{
			FSFUtils::Log("[USFFadeHandler::Fade]: Fading out (to Black)", false);
		}
		else
		{
			FSFUtils::Log("[USFFadeHandler::Fade]: Fading in (from Black)", false);
		}
	}

	const UWorld* World = GameInstance->GetWorld();
	if (World)
	{
		USFGlobalFadeGameViewportClient* GameViewportClient = Cast<USFGlobalFadeGameViewportClient>(
			World->GetGameViewport());
		if (GameViewportClient)
		{
			GameViewportClient->Fade(Duration, bToBlack, FadeColor);
		}
		else
		{
			FSFUtils::OpenMessageBox(
				"GameViewport is not set to USFGlobalFadeGameViewportClient, Study Framework will not work correctly",
				true);
		}
	}
}


bool USFFadeHandler::GetIsFading() const
{
	return FadeState != EFadeState::NotFading;
}

APlayerCameraManager* USFFadeHandler::GetCameraManager() const
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GameInstance->GetWorld(), 0);

	if (CameraManager == nullptr)
	{
		FSFUtils::Log("[USFFadeHandler::GetCameraManager()]: Cannot get CameraManager", true);
	}

	return CameraManager;
}


void USFFadeHandler::SetGameInstance(USFGameInstance* Instance)
{
	GameInstance = Instance;
}

void USFFadeHandler::SetFadeDuration(const float FadeDurationN)
{
	FadeDuration = FadeDurationN;
}

void USFFadeHandler::SetFadedOutDuration(const float FadeOutWaitN)
{
	FadeOutWait = FadeOutWaitN;
}

void USFFadeHandler::SetFadeColor(FLinearColor Color)
{
	FadeColor = Color;
}

void USFFadeHandler::SetInitialFadedOut(const bool bFadedOut)
{
	bIsFadedOut = bFadedOut;
	Fade(0.0f, bFadedOut);
}

void USFFadeHandler::SetTimerForNextTick(const float TimeToWait)
{
	FSFUtils::Log(
		"[USFFadeHandler::SetTimerForNextTick()]: Setting Timer for " + FString::SanitizeFloat(TimeToWait, 4) +
		" seconds", false);
	GameInstance->GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USFFadeHandler::Tick, 0.1f, true,
	                                                     TimeToWait);
}
