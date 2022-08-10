// Fill out your copyright notice in the Description page of Project Settings.


#include "Logging/SFLoggingBPLibrary.h"

#include "IUniversalLogging.h"
#include "SFGameInstance.h"
#include "Logging/SFLoggingUtils.h"
#include "Logging/SFLogObject.h"

 void USFLoggingBPLibrary::LogToFile() {
    if(!USFGameInstance::Get() || !USFGameInstance::Get()->GetLogObject())
    {
        return;
    }
    USFLogObject* LogObject = USFGameInstance::Get()->GetLogObject();
 	if (LogObject->LogThis == false) {
 		return;
 	}
    for (auto& ActorLoggingInfo : LogObject->LoggingInfo) {
        if (ActorLoggingInfo.LogNextTick == true) {
            ActorLoggingInfo.LogNextTick = false;
            float TimeSinceStart = ActorLoggingInfo.ActorToLog->GetWorld()->GetTimeSeconds();
            FString TimeSinceStartString = FString::SanitizeFloat(TimeSinceStart);
            FString out = "" + TimeSinceStartString +
                "\t" + ActorLoggingInfo.LogName +
                "\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().X) +
                "\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Y) +
                "\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorLocation().Z) +
                "\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Pitch) +
                "\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Yaw) +
                "\t" + FString::Printf(TEXT("%.3f"), ActorLoggingInfo.ActorToLog->GetActorRotation().Roll);

            if (UniLog.GetLogStream("PositionLog"))
            {
                UniLog.Log(out, "PositionLog");
            }        	
        }
        if (((FDateTime::Now() - ActorLoggingInfo.TimeStorage).GetTotalMilliseconds() > ActorLoggingInfo.LogTimer) || (ActorLoggingInfo.LogTimer == 0)) {
            ActorLoggingInfo.TimeStorage = FDateTime::Now();
            ActorLoggingInfo.LogNextTick = true;
        }
    }
 }
 void USFLoggingBPLibrary::LogData(const FString& DependentVariableName, const FString& Value)
 {
     USFCondition* CurrCondition = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition();
     if (!CurrCondition->StoreDependentVariableData(DependentVariableName, Value))
     {
         FSFLoggingUtils::Log(
             "Cannot log data '" + Value + "' for dependent variable '" + DependentVariableName +
             "' since it does not exist for this condition!", true);
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

 void USFLoggingBPLibrary::AddActor(AActor* Actor, int32 LogTimer, FString LogName)
 {
     if (USFGameInstance::Get() && USFGameInstance::Get()->GetLogObject())
     {
         USFGameInstance::Get()->GetLogObject()->AddActorWithName(Actor, LogTimer, LogName);
     }
 }
