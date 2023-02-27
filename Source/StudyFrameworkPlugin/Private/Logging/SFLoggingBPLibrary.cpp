// Fill out your copyright notice in the Description page of Project Settings.


#include "Logging/SFLoggingBPLibrary.h"

#include "IUniversalLogging.h"
#include "SFGameInstance.h"
#include "Logging/SFLoggingUtils.h"
#include "Logging/SFLogObject.h"

void USFLoggingBPLibrary::LogData(const FString& DependentVariableName, const FString& Value)
{
  USFGameInstance::Get()->LogData(DependentVariableName, Value);
}

void USFLoggingBPLibrary::LogTrialData(const FString& DependentVariableName, const TArray<FString>& Values)
{
  USFGameInstance::Get()->LogTrialData(DependentVariableName, Values);
}

void USFLoggingBPLibrary::LogComment(const FString& Comment, bool bAlsoLogToHUD /*=false*/)
{
	USFGameInstance::Get()->LogComment(Comment, bAlsoLogToHUD);
}

void USFLoggingBPLibrary::LogToHUD(const FString& String)
{
  USFGameInstance::Get()->LogToHUD(String);
}

void USFLoggingBPLibrary::AddActor(AActor* Actor, int32 LogTimer, FString LogName)
{
	if (USFGameInstance::Get() && USFGameInstance::Get()->GetLogObject())
	{
		USFGameInstance::Get()->GetLogObject()->AddComponentWithName(Actor->GetRootComponent(), LogTimer, LogName);
	}
}

void USFLoggingBPLibrary::AddComponent(USceneComponent* Component, int32 LogTimer, FString LogName)
{
	if (USFGameInstance::Get() && USFGameInstance::Get()->GetLogObject())
	{
		USFGameInstance::Get()->GetLogObject()->AddComponentWithName(Component, LogTimer, LogName);
	}
}

void USFLoggingBPLibrary::RemoveActor(AActor* Actor)
{
	if (USFGameInstance::Get() && USFGameInstance::Get()->GetLogObject())
  {
    USFGameInstance::Get()->GetLogObject()->RemoveEntryByComponent(Actor->GetRootComponent());
  }
}

void USFLoggingBPLibrary::RemoveComponent(USceneComponent* Component)
{
  if (USFGameInstance::Get() && USFGameInstance::Get()->GetLogObject())
  {
    USFGameInstance::Get()->GetLogObject()->RemoveEntryByComponent(Component);
  }
}
