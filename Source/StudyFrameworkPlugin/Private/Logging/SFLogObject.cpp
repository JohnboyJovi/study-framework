#include "Logging/SFLogObject.h"
#include "Engine.h"
#include "IUniversalLogging.h"
#include "SFGameInstance.h"

USFLogObject::USFLogObject() {
	ActorLoggingInfoArray.Init(FActorLoggingInformation(0, nullptr, ""), 0);
}

void USFLogObject::AddActor(AActor* Actor, int32 LogTimer) {
	// Actor already in list -> overwrite existing Entry
	if(GetEntryByActor(Actor))
	{
		GetEntryByActor(Actor)->LogTimer = LogTimer;
		return;
	}
	ActorLoggingInfoArray.Add(FActorLoggingInformation(LogTimer, Actor, Actor->GetFName().ToString()));
}

void USFLogObject::AddActorWithName(AActor* Actor, int32 LogTimer, FString LogName) {
	// Actor already in list -> overwrite existing Entry
	if (GetEntryByActor(Actor))
	{
		GetEntryByActor(Actor)->LogTimer = LogTimer;
		GetEntryByActor(Actor)->LogName = LogName;
		return;
	}
	ActorLoggingInfoArray.Add(FActorLoggingInformation(LogTimer, Actor, LogName));
	UE_LOG(LogTemp, Display, TEXT("Added Actor %s"), *LogName)
}

void USFLogObject::AddActors(TArray<AActor*> ActorArray, int32 LogTimer) {
	for (int32 Index = 0; Index != ActorArray.Num(); ++Index) {
		AddActor(ActorArray[Index], LogTimer);
	}
}


FActorLoggingInformation* USFLogObject::GetEntryByActor(const AActor* Actor)
{
	for (int i = 0; i< ActorLoggingInfoArray.Num(); i++)
	{
		if(ActorLoggingInfoArray[i].ActorToLog == Actor)
		{
			return &ActorLoggingInfoArray[i];
		}
	}
	return nullptr;
}

void USFLogObject::RemoveEntryByActor(const AActor* Actor)
{
	for (int i = 0; i < ActorLoggingInfoArray.Num(); i++)
	{
		if (ActorLoggingInfoArray[i].ActorToLog == Actor)
		{
			ActorLoggingInfoArray.RemoveAt(i);
		}
	}
}

void USFLogObject::Initialize() {
	StaticDateTime = FDateTime::Now();
	ProbandID = 0;
	ActorLoggingInfoArray.SetNum(0, true);
}

// NOTE: When changing header row, update output (see below)
void USFLogObject::LogHeaderRows() {
	FString PositionLogHeader = "#" + FString("ElapsedTime") +
		"\t" + FString("Actor") +
		"\t" + FString("Condition") +
		"\t" + FString("Location-X") +
		"\t" + FString("Location-Y") +
		"\t" + FString("Location-Z") +
		"\t" + FString("Rotation-Pitch") +
		"\t" + FString("Rotation-Yaw") +
		"\t" + FString("Rotation-Yaw");
	UniLog.Log(PositionLogHeader, "PositionLog");
}

void USFLogObject::LoggingLoopsLogToFile() {
	if (!USFGameInstance::Get() || !USFGameInstance::Get()->GetLogObject())
	{
		return;
	}
	USFLogObject* LogObject = USFGameInstance::Get()->GetLogObject();
	for (auto& ActorLoggingInfo : LogObject->ActorLoggingInfoArray) {
		if (ActorLoggingInfo.LogNextTick == true) {
			ActorLoggingInfo.LogNextTick = false;
			//When starting in Debug-Mode (i.e. not through the HUD) no condition is defined. 
			FString currentCondition = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition() ?
				USFGameInstance::Get()->GetParticipant()->GetCurrentCondition()->UniqueName :
				FString("Debug");
			// NOTE: When changing output, update header row (see above)
			FString out = "#" + USFGameInstance::Get()->GetParticipant()->GetCurrentTime() +
				"\t" + ActorLoggingInfo.LogName +
				"\t" + currentCondition +
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


void USFLogObject::SetLoggingLoopsActive(bool LoggingLoopsActive) {
	bLoggingLoopsActive = LoggingLoopsActive;
}

bool USFLogObject::GetLoggingLoopsActive() {
	return bLoggingLoopsActive;
}
