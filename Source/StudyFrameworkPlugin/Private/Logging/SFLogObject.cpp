#include "Logging/SFLogObject.h"
#include "Engine.h"
#include "SFGameInstance.h"

USFLogObject::USFLogObject() {
	LoggingInfo.Init(FActorLoggingInformation(0, nullptr, ""), 0);
}

void USFLogObject::AddActor(AActor* Actor, int32 LogTimer) {
	// Actor already in list -> overwrite existing Entry
	if(GetEntryByActor(Actor))
	{
		GetEntryByActor(Actor)->LogTimer = LogTimer;
		return;
	}
	LoggingInfo.Add(FActorLoggingInformation(LogTimer, Actor, Actor->GetFName().ToString()));
}

void USFLogObject::AddActorWithName(AActor* Actor, int32 LogTimer, FString LogName) {
	// Actor already in list -> overwrite existing Entry
	if (GetEntryByActor(Actor))
	{
		GetEntryByActor(Actor)->LogTimer = LogTimer;
		GetEntryByActor(Actor)->LogName = LogName;
		return;
	}
	LoggingInfo.Add(FActorLoggingInformation(LogTimer, Actor, LogName));
	UE_LOG(LogTemp, Display, TEXT("Added Actor %s"), *LogName)
}

void USFLogObject::AddActors(TArray<AActor*> ActorArray, int32 LogTimer) {
	for (int32 Index = 0; Index != ActorArray.Num(); ++Index) {
		AddActor(ActorArray[Index], LogTimer);
	}
}


FActorLoggingInformation* USFLogObject::GetEntryByActor(const AActor* Actor)
{
	for (int i = 0; i<LoggingInfo.Num(); i++)
	{
		if(LoggingInfo[i].ActorToLog == Actor)
		{
			return &LoggingInfo[i];
		}
	}
	return nullptr;
}

void USFLogObject::RemoveEntryByActor(const AActor* Actor)
{
	for (int i = 0; i < LoggingInfo.Num(); i++)
	{
		if (LoggingInfo[i].ActorToLog == Actor)
		{
			LoggingInfo.RemoveAt(i);
		}
	}
}

void USFLogObject::Initialize() {
	LogThis = true;
	StaticDateTime = FDateTime::Now();
	ProbandID = 0;
	LoggingInfo.SetNum(0, true);
}