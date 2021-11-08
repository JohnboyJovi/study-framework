#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"

#include "SFDefines.h"

class USFGameInstance;
class APlayerCameraManager;

#include "SFFadeHandler.generated.h"


UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFFadeHandler : public UObject
{
	GENERATED_BODY()

public:
	void Tick();

	void FadeToLevel(FString LevelName, bool bStartFadedOut = false);
	void FadeIn();
	float FadeTimeRemaining() const;


	bool GetIsFading() const;
	APlayerCameraManager* GetCameraManager() const;

	void SetGameInstance(USFGameInstance* Instance);
	void SetFadeDuration(float FadeDurationN);
	void SetFadedOutDuration(float FadeOutWaitN);
	void SetFadeColor(FLinearColor Color);
	void SetInitialFadedOut(bool bFadedOut);


private:
	void SetTimerForNextTick(float TimeToWait = 0.0f); // Starts Tick Function in x sec

	void Fade(float Duration, bool bToBlack) const;

	// virtual void BeforeLevelUnload();
	// virtual void OnNewLevelLoaded();


private:

	UPROPERTY()
	float FadeDuration = 2.0f; // Time the fade takes

	UPROPERTY()
	float FadeOutWait = 1.0f; // Time to wait when level is loaded


	UPROPERTY()
	bool bIsFadedOut = true;

	UPROPERTY()
	TEnumAsByte<EFadeState> FadeState = EFadeState::NotFading;

	UPROPERTY()
	FLinearColor FadeColor = FLinearColor::Black;

	UPROPERTY()
	FString NewLevelName;

	UPROPERTY()
	USFGameInstance* GameInstance;

	UPROPERTY()
	FTimerHandle TimerHandle;
};
