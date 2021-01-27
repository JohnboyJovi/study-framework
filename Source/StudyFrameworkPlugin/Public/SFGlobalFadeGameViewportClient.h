// Copyright 2015 Moritz Wundke. All Rights Reserved.
// Released under MIT

#pragma once

#include "Engine/GameViewportClient.h"
#include "SFGlobalFadeGameViewportClient.generated.h"

/**
 * A simple UGameViewportClient used to handle a global fade in/out
 
 * Always starts faded out
 
 */

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFGlobalFadeGameViewportClient : public UGameViewportClient
{
    GENERATED_BODY()

public:

    /**
     * Called after rendering the player views and HUDs to render menus, the console, etc.
     * This is the last rendering call in the render loop
     *
     * @param Canvas        The canvas to use for rendering.
     */
    virtual void PostRender(UCanvas* Canvas) override;

    /** Clear fading state */
    virtual void ClearFade();

    /** Used for Fade to and from black */
    virtual void Fade(const float Duration, const bool bToBlackN, const FLinearColor FadeColorN = FLinearColor::Black);

    /** Get how many seconds are left */
    float FadeTimeRemaining();

    /** Does the actual screen fading */
    void DrawScreenFade(UCanvas* Canvas);

private:

    // Values used by our screen fading
    bool bFading = true;
    bool bToBlack = true; // Fading to black will be applied even if alpha is 1
    float FadeStartTime = 0.0f;
    float FadeDuration = 0.0f;
    FLinearColor FadeColor = FLinearColor::Black;
};
