#include "Logging/SFLogObject.h"
#include "Engine.h"
#include "IUniversalLogging.h"
#include "SFGameInstance.h"
#include "Logging/SFLoggingUtils.h"

USFLogObject::USFLogObject() {
	ComponentLoggingInfoArray.Init(FComponentLoggingInformation(0, nullptr, ""), 0);
}

void USFLogObject::AddComponentWithName(USceneComponent* Component, int32 LogTimer, FString LogName) {
	// Component already in list -> overwrite existing Entry
	if (GetEntryByComponent(Component))
	{
		GetEntryByComponent(Component)->LogTimer = LogTimer;
		GetEntryByComponent(Component)->LogName = LogName;
		return;
	}
	ComponentLoggingInfoArray.Add(FComponentLoggingInformation(LogTimer, Component, LogName));
	FSFLoggingUtils::Log("PositionLogging: Added Component "+ LogName + " (" + Component->GetReadableName() +") " + " to PositionLogging Array", false);
}

FComponentLoggingInformation* USFLogObject::GetEntryByComponent(const USceneComponent* Component)
{
	for (int i = 0; i< ComponentLoggingInfoArray.Num(); i++)
	{
		if(ComponentLoggingInfoArray[i].ComponentToLog == Component)
		{
			return &ComponentLoggingInfoArray[i];
		}
	}
	return nullptr;
}

void USFLogObject::RemoveEntryByComponent(const USceneComponent* Component)
{
	for (int i = 0; i < ComponentLoggingInfoArray.Num(); i++)
	{
		if (ComponentLoggingInfoArray[i].ComponentToLog == Component)
		{
			ComponentLoggingInfoArray.RemoveAt(i);
		}
	}
}

void USFLogObject::Initialize() {
	StaticDateTime = FDateTime::Now();
	ProbandID = 0;
	ComponentLoggingInfoArray.SetNum(0, true);
}

// NOTE: When changing header row, update output (see below)
void USFLogObject::WritePositionLogHeaderRow() {
	FString PositionLogHeader = FString("ElapsedTime") +
		"\t" + FString("LogName") +
		"\t" + FString("Condition") +
		"\t" + FString("Location-X") +
		"\t" + FString("Location-Y") +
		"\t" + FString("Location-Z") +
		"\t" + FString("Rotation-Pitch") +
		"\t" + FString("Rotation-Yaw") +
		"\t" + FString("Rotation-Yaw");
	UniLog.Log(PositionLogHeader, "PositionLog");
}

void USFLogObject::WritePositionLogToFile() {
	if (!USFGameInstance::Get() || !USFGameInstance::Get()->GetLogObject())
	{
		return;
	}
	USFLogObject* LogObject = USFGameInstance::Get()->GetLogObject();
	for (auto& ComponentLoggingInfo : LogObject->ComponentLoggingInfoArray) {
		if (ComponentLoggingInfo.LogNextTick == true) {
			ComponentLoggingInfo.LogNextTick = false;
			//When starting in Debug-Mode (i.e. not through the HUD) no condition is defined. 
			FString CurrentCondition = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition() ?
				USFGameInstance::Get()->GetParticipant()->GetCurrentCondition()->UniqueName :
				FString("Debug");
			// NOTE: When changing output, update header row (see above)

			FString out = USFGameInstance::Get()->GetParticipant()->GetCurrentTime() +
				"\t" + ComponentLoggingInfo.LogName +
				"\t" + CurrentCondition +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentLocation().X) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentLocation().Y) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentLocation().Z) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentRotation().Pitch) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentRotation().Yaw) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentRotation().Roll);
			if (UniLog.GetLogStream("PositionLog"))
			{
				UniLog.Log(out, "PositionLog");
			}
		}
		if (((FDateTime::Now() - ComponentLoggingInfo.TimeStorage).GetTotalMilliseconds() > ComponentLoggingInfo.LogTimer) || (ComponentLoggingInfo.LogTimer == 0)) {
			ComponentLoggingInfo.TimeStorage = FDateTime::Now();
			ComponentLoggingInfo.LogNextTick = true;
		}
	}
}

// NOTE: When changing header row, update output (see below)
void USFLogObject::WriteGazeTrackingLogHeaderRow() {
	FString GazeTrackingLogHeader = FString("ElapsedTime") +
		"\t" + FString("Condition") +
		"\t" + FString("TrackingEyes") +
		"\t" + FString("GazeTarget") +
		"\t" + FString("Gaze-Origin-X-Y-Z") +
		"\t" + FString("Gaze-Direction-X-Y-Z");
	UniLog.Log(GazeTrackingLogHeader, "GazeTrackingLog");
}

void USFLogObject::WriteGazeTrackingLogToFile() {
	if (!USFGameInstance::Get() || !USFGameInstance::Get()->GetGazeTracker())
	{
		return;
	}
	USFGazeTracker* GazeTracker = USFGameInstance::Get()->GetGazeTracker();
	FString GazeTarget = GazeTracker->GetCurrentGazeTarget() == "" ? "-" : GazeTracker->GetCurrentGazeTarget();
	//When starting in Debug-Mode (i.e. not through the HUD) no condition is defined. 
	FString CurrentCondition = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition() ?
								USFGameInstance::Get()->GetParticipant()->GetCurrentCondition()->UniqueName :
								FString("Debug");
	FString isTrackingEyes = USFGameInstance::Get()->GetGazeTracker()->IsTrackingEyes() ? "1" : "0";
	// NOTE: When changing output, update header row (see above)
	FString out = USFGameInstance::Get()->GetParticipant()->GetCurrentTime() +
		"\t" + CurrentCondition +
		"\t" + isTrackingEyes +
		"\t" + GazeTarget +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Origin.X) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Origin.Y) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Origin.Z) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Direction.X) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Direction.Y) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Direction.Z);
	if (UniLog.GetLogStream("GazeTrackingLog"))
	{
		UniLog.Log(out, "GazeTrackingLog");
	}
}

void USFLogObject::SetLoggingLoopsActive(bool LoggingLoopsActive) {
	bLoggingLoopsActive = LoggingLoopsActive;
	FString StateToLog = LoggingLoopsActive ? "Enabled" : "Disabled";
	FSFLoggingUtils::Log("PositionLogging: " + StateToLog + " position logging loops.");
}

bool USFLogObject::GetLoggingLoopsActive() {
	return bLoggingLoopsActive;
}
