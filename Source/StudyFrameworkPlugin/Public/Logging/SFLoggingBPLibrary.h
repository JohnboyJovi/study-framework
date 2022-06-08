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
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Log To File", Keywords = "SFLogging"), Category = "SFLogging")
		static void LogToFile();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Actor to Logging", Keywords = "SFLogging"), Category = "SFLogging")
		static void AddActor(AActor* Actor, int32 LogTimer, FString LogName);
};
