// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/SFFadeHandler.h"

#if WITH_EDITOR
#include "EditorWidgets/Public/EditorWidgets.h"
#endif
#include "Kismet/GameplayStatics.h"
#include "SFGameInstance.h"

#include "Help/SFUtils.h"

#include "TimerManager.h"						// For Tick Timer
#include "HUD/SFGlobalFadeGameViewportClient.h"		// For Fade
#include "Logging/SFLoggingUtils.h"


TSharedPtr<FJsonObject> FFadeConfig::GetAsJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	Json->SetBoolField("StartFadedOut", bStartFadedOut);
	Json->SetNumberField("FadeDuration", FadeDuration);
	Json->SetNumberField("FadeOutDuration", FadedOutDuration);
	Json->SetStringField("FadeColor", FadeColor.ToString());
	return Json;
}

void FFadeConfig::FromJson(TSharedPtr<FJsonObject> Json)
{
	bStartFadedOut = Json->GetBoolField("StartFadedOut");
	FadeDuration = Json->GetNumberField("FadeDuration");
	FadedOutDuration = Json->GetNumberField("FadeOutDuration");
	FadeColor.InitFromString(Json->GetStringField("FadeColor"));
}

void USFFadeHandler::Tick()
{
	if (FadeState == EFadeState::NotFading || (FadeTimeRemaining() > 0.0f && FadeState != EFadeState::WaitForLevelLoaded)
	)
	{
		return;
	}

	USFGameInstance* GameInstance = USFGameInstance::Get();

	switch (FadeState)
	{
		// Its Faded out, Load new Level now
	case EFadeState::FadingOut:
		if(!NewLevelName.IsEmpty())
		{
			FSFLoggingUtils::Log("[USFFadeHandler::Tick()]: Opening Level now", false);
			UGameplayStatics::OpenLevel(GameInstance->GetWorld(), *NewLevelName, false);
			SetTimerForNextTick();
			FadeState = EFadeState::WaitForLevelLoaded;
		}
		else if(USFGameInstance::Get()->HasEnded())
		{
			//we quit the game
			UKismetSystemLibrary::QuitGame(USFGameInstance::Get()->GetWorld(), nullptr, EQuitPreference::Quit, false);
		}
		//else just do nothing so we stay faded out NewLevelName=="" is the case for a simple FadeOut()
		break;

		// Its Faded out, New Level is loaded, prepare it
	case EFadeState::WaitForLevelLoaded:
		FSFLoggingUtils::Log("[USFFadeHandler::Tick()]: Set Scene to black on new Level (pre) loaded", false);
		Fade(0.0, true);
		FadeState = EFadeState::WaitForTimerFadedOut;
		SetTimerForNextTick(FadeConfig.FadedOutDuration);
		break;

		// Its Faded out, Timer for Fading in is done
	case EFadeState::WaitForTimerFadedOut:
		GameInstance->OnLevelLoaded();
		FSFLoggingUtils::Log("[USFFadeHandler::Tick()]: Fading in now", false);
		FadeIn();
		break;

		// Its Faded in, everything done, cleanup
	case EFadeState::FadingIn:
		FSFLoggingUtils::Log("[USFFadeHandler::Tick()]: Cleaning up", false);
		GameInstance->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		GameInstance->OnFadedIn();
		FadeState = EFadeState::NotFading;
		GameInstance->GetLogObject()->SetLoggingLoopsActive(true);
		break;

	case EFadeState::NotFading:
	default:
		FSFLoggingUtils::Log("[USFFadeHandler::Tick()]: in Default or Not Fading switch case", true);
		return;
	}
}

void USFFadeHandler::FadeToLevel(const FString& NextLevelName, bool bForceFade, const bool bStartFadeFadedOut)
{
	if (GetCameraManager() == nullptr)
	{
		return;
	}

	if (GetIsFading())
	{
		return;
	}
	// Pause Logging Loops (e.g. position logging) between conditions
	USFGameInstance::Get()->GetLogObject()->SetLoggingLoopsActive(false);
	// reset logging info array for new condition, because actors in list will be destroyed and garbage collected when new level is loaded
	USFGameInstance::Get()->GetLogObject()->RemoveAllTrackedComponents();
	FString CurrentLevelName = USFGameInstance::Get()->GetWorld()->GetMapName();
	CurrentLevelName.RemoveFromStart(USFGameInstance::Get()->GetWorld()->StreamingLevelsPrefix);
	FSFLoggingUtils::Log(
		"[USFFadeHandler::FadeToLevel()]: Fading From level (" + CurrentLevelName + ") to level (" + NextLevelName + ")", false);

	if (bStartFadeFadedOut || bIsFadedOut)
	{
		//we only need to fade in
		if (bIsFadedOut)
		{
			bIsFadedOut = false;
		}
		Fade(0.0f, true);
		FadeState = EFadeState::FadingOut;
	}
	else if (USFGameInstance::Get()->GetStudySetup()->bNoFadingOnSameMap 
		&& CurrentLevelName == FPackageName::GetShortName(NextLevelName)
		&& !bForceFade)
	{
		//bNoFadingOnSameMap and fade to same map, so no fading, but pretend we "faded in"
		FadeState = EFadeState::FadingIn;
		FSFLoggingUtils::Log(
			"[USFFadeHandler::FadeToLevel()]: Actually not fading but just calling OnFaded in (bNoFadingOnSameMap)", false);
	}
	else
	{
		//fade out and then in again
		Fade(FadeConfig.FadeDuration, true);
		FadeState = EFadeState::FadingOut;
	}


	NewLevelName = NextLevelName;
	SetTimerForNextTick();
}


void USFFadeHandler::FadeIn()
{
	if (!GetCameraManager())
	{
		return;
	}
	Fade(FadeConfig.FadeDuration, false);
	FadeState = EFadeState::FadingIn;
	SetTimerForNextTick();
}

void USFFadeHandler::FadeOut()
{
	FadeToLevel("", true);
}


float USFFadeHandler::FadeTimeRemaining() const
{
	const UWorld* World = USFGameInstance::Get()->GetWorld();
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
			FSFLoggingUtils::Log("[USFFadeHandler::Fade]: Fading out (to Black)", false);
		}
		else
		{
			FSFLoggingUtils::Log("[USFFadeHandler::Fade]: Fading in (from Black)", false);
		}
	}

	if (USFGameInstance::IsInitialized() && USFGameInstance::Get()->GetWorld())
	{
		USFGlobalFadeGameViewportClient* GameViewportClient = Cast<USFGlobalFadeGameViewportClient>(
			USFGameInstance::Get()->GetWorld()->GetGameViewport());
		if (GameViewportClient)
		{
			GameViewportClient->Fade(Duration, bToBlack, FadeConfig.FadeColor);
		}
	}
}


bool USFFadeHandler::GetIsFading() const
{
	return FadeState != EFadeState::NotFading;
}

bool USFFadeHandler::GetIsFadedOutWaitingForLevel() const
{
	return FadeTimeRemaining() == 0.0f && FadeState == EFadeState::FadingOut;
}

APlayerCameraManager* USFFadeHandler::GetCameraManager() const
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(USFGameInstance::Get()->GetWorld(), 0);

	if (CameraManager == nullptr)
	{
		FSFLoggingUtils::Log("[USFFadeHandler::GetCameraManager()]: Cannot get CameraManager", false);
	}

	return CameraManager;
}



void USFFadeHandler::SetFadeDuration(const float FadeDurationN)
{
	FadeConfig.FadeDuration = FadeDurationN;
}

void USFFadeHandler::SetFadedOutDuration(const float FadeOutWaitN)
{
	FadeConfig.FadedOutDuration = FadeOutWaitN;
}

void USFFadeHandler::SetFadeColor(FLinearColor Color)
{
	FadeConfig.FadeColor = Color;
}

void USFFadeHandler::SetInitialFadedOut(const bool bFadedOut)
{
	bIsFadedOut = bFadedOut;
	Fade(0.0f, bFadedOut);
	FadeConfig.bStartFadedOut = bFadedOut;
}


FFadeConfig USFFadeHandler::GetFadeConfig() const
{
	return FadeConfig;
}

void USFFadeHandler::SetFadeConfig(FFadeConfig InFadeConfig)
{
	FadeConfig = InFadeConfig;
}

void USFFadeHandler::SetTimerForNextTick(const float TimeToWait)
{
	//FSFLoggingUtils::Log(
	//	"[USFFadeHandler::SetTimerForNextTick()]: Setting Timer for " + FString::SanitizeFloat(TimeToWait, 4) +
	//	" seconds", false);
	USFGameInstance::Get()->GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USFFadeHandler::Tick, 0.1f, true,
	                                                     TimeToWait);
}

void USFFadeHandler::SetLevelToLoad(const FString& LevelName)
{
	if (!GetIsFadedOutWaitingForLevel()) return;
	NewLevelName = LevelName;
}
