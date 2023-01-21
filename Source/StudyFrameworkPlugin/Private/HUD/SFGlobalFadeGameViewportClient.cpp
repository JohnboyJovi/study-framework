// Copyright 2015 Moritz Wundke. All Rights Reserved.
// Released under MIT

#include "HUD/SFGlobalFadeGameViewportClient.h"
#include "Help/SFUtils.h"


#include "Engine/Canvas.h"
#include "HUD/SFMasterHUD.h"
#include "Utility/VirtualRealityUtilities.h"

// Link to the Tutorial of the manual Viewport Client
// https://nerivec.github.io/old-ue4-wiki/pages/global-fade-in-out.html

// Link for setting manual Viewport Client
// https://forums.unrealengine.com/unreal-engine/announcements-and-releases/1745504-a-new-community-hosted-unreal-engine-wiki


void USFGlobalFadeGameViewportClient::PostRender(UCanvas* Canvas)
{
	Super::PostRender(Canvas);

	// Fade if requested, you could use the same DrawScreenFade method from any canvas such as the HUD
	if (bFading)
	{
		DrawScreenFade(Canvas);
	}
}

void USFGlobalFadeGameViewportClient::ClearFade()
{
	bFading = false;
}

void USFGlobalFadeGameViewportClient::Fade(const float Duration, const bool bToBlackN, const FLinearColor FadeColorN)
{
	bFading = true;
	bToBlack = bToBlackN;
	FadeDuration = Duration;
	FadeStartTime = FPlatformTime::Seconds();
	FadeColor = FadeColorN;
}

float USFGlobalFadeGameViewportClient::FadeTimeRemaining()
{
	if (bFading)
	{
		const double Time = FPlatformTime::Seconds();
		const float TimeLeft = FadeDuration - (Time - FadeStartTime);
		if (TimeLeft > 0.0f)
		{
			return TimeLeft;
		}
	}
	return 0.0f;
}

void USFGlobalFadeGameViewportClient::DrawScreenFade(UCanvas* Canvas)
{
	if (bFading)
	{
		float Alpha = 1.0f;
		if (FadeDuration > 0.0f)
		{
			Alpha = FadeTimeRemaining() / FadeDuration;
		}

		if (Alpha < 1.0f)
		{
			FLinearColor FadeColorTmp = FadeColor;
			FadeColorTmp.A = bToBlack ? 1 - Alpha : Alpha;

			ASFMasterHUD* MasterHUD = nullptr;
			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			if (PlayerController)
			{
				MasterHUD = Cast<ASFMasterHUD>(PlayerController->GetHUD());
			}

			if (MasterHUD && UVirtualRealityUtilities::IsMaster())
			{
				//if we use the HUD let it do the fading, so it can still be seen when faded out
				MasterHUD->SetBackgroundAlpha(FadeColorTmp.A);
			}
			else
			{
				const FColor OldColor = Canvas->DrawColor;
				Canvas->DrawColor = FadeColorTmp.ToFColor(true);
				// TheJamsh: "4.10 cannot convert directly to FColor, so need to use FLinearColor::ToFColor() :)
				Canvas->DrawTile(Canvas->DefaultTexture, 0, 0, Canvas->ClipX, Canvas->ClipY, 0, 0,
				                 Canvas->DefaultTexture->GetSizeX(), Canvas->DefaultTexture->GetSizeY());
				Canvas->DrawColor = OldColor;
			}
		}
	}
}
