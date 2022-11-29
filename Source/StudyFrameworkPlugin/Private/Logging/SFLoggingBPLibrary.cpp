// Fill out your copyright notice in the Description page of Project Settings.


#include "Logging/SFLoggingBPLibrary.h"

#include "IUniversalLogging.h"
#include "SFGameInstance.h"
#include "Logging/SFLoggingUtils.h"
#include "Logging/SFLogObject.h"

 void USFLoggingBPLibrary::LogData(const FString& DependentVariableName, const FString& Value)
 {
     USFCondition* CurrCondition = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition();
     if (!CurrCondition->StoreDependentVariableData(DependentVariableName, Value))
     {
         return;
     }
     LogComment("Recorded " + DependentVariableName + ": " + Value);

     //the data is stored in the phase long table on SetCondition() or EndStudy()
 }

 void USFLoggingBPLibrary::LogComment(const FString& Comment, bool AlsoLogToHUD /*=false*/)
 {
     UniLog.Log("#" + USFGameInstance::Get()->GetParticipant()->GetCurrentTime() + ": " + Comment, "ParticipantLog");
     FSFLoggingUtils::Log("Logged Comment: " + Comment);
     if (AlsoLogToHUD)
     {
         USFGameInstance::Get()->LogToHUD(Comment);
     }
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
