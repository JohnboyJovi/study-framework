// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SFLoggingBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFLoggingBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// For usability it is possible to "add actors" to position logging,
	// under the hood the root component of that actor is added to logging
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Actor to Logging", Keywords = "SFLogging"), Category = "SFLogging")
		static void AddActor(AActor* Actor, int32 LogTimer, FString LogName);

	//Add component to position logging
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Component to Logging", Keywords = "SFLogging"), Category = "SFLogging")
		static void AddComponent(USceneComponent* Component, int32 LogTimer, FString LogName);

	//Log Data collected for a DependentVariable in this condition
	UFUNCTION(BlueprintCallable)
		static void LogData(const FString& DependentVariableName, const FString& Value);

	//Log a comment (with a timestamp) to store events or user action etc. in the participant's log file
	//and optionally log to HUD (default=false)
	UFUNCTION(BlueprintCallable)
		static void LogComment(const FString& Comment, bool AlsoLogToHUD = false);

	//Log something to the HUD, for debugging or giving information to the experimentator
	UFUNCTION(BlueprintCallable)
		static void LogToHUD(const FString& String);
};
