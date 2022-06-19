// Fill out your copyright notice in the Description page of Project Settings.


#include "Logging/SFLoggingBPLibrary.h"

#include "IUniversalLogging.h"
#include "SFGameInstance.h"
 #include "SFPlugin.h"
 #include "Logging/SFLogObject.h"
 #include "Misc/FileHelper.h"

 void USFLoggingBPLibrary::LogToFile() {
    //UniLog.Log("called LogToFile");
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
            //Automatically adds newline
            //TODO: implement LogStream for more coordinated logging, use that later
            //TODO: configure options like: "per session" -> probably useful in study-fw scenario
            UniLog.Log(out);
        }
        if (((FDateTime::Now() - ActorLoggingInfo.TimeStorage).GetTotalMilliseconds() > ActorLoggingInfo.LogTimer) || (ActorLoggingInfo.LogTimer == 0)) {
            ActorLoggingInfo.TimeStorage = FDateTime::Now();
            ActorLoggingInfo.LogNextTick = true;
        }
    }
 }
 void USFLoggingBPLibrary::AddActor(AActor* Actor, int32 LogTimer, FString LogName)
 {
     if (USFGameInstance::Get() && USFGameInstance::Get()->GetLogObject())
     {
         USFGameInstance::Get()->GetLogObject()->AddActorWithName(Actor, LogTimer, LogName);
     }
 }