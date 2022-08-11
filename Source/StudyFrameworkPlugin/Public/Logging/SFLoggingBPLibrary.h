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
	UFUNCTION()
		static void LogHeaderRows();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Log To File", Keywords = "SFLogging"), Category = "SFLogging")
		static void LogToFile();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Actor to Logging", Keywords = "SFLogging"), Category = "SFLogging")
		static void AddActor(AActor* Actor, int32 LogTimer, FString LogName);

	//Log Data collected for a DependentVariable in this condition
	UFUNCTION(BlueprintCallable)
		static void LogData(const FString& DependentVariableName, const FString& Value);

	//Log a comment (with a timestamp) to store events or user action etc. in the participant's log file
	//and optionally log to HUD (default=false)
	UFUNCTION(BlueprintCallable)
		static void LogComment(const FString& Comment, bool AlsoLogToHUD = false);
};
