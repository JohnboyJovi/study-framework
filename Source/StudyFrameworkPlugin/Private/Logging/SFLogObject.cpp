#include "Logging/SFLogObject.h"
#include "Engine.h"
#include "IUniversalLogging.h"
#include "SFGameInstance.h"
#include "Logging/SFLogCustomDataComponent.h"
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

void USFLogObject::RemoveAllTrackedComponents()
{
	ComponentLoggingInfoArray.Empty();
}

void USFLogObject::Initialize() {
	StaticDateTime = FDateTime::Now();
	ComponentLoggingInfoArray.SetNum(0, true);
	bGazingLoggingFileCreated = false;
	bPositionLoggingFileCreated = false;
}

// NOTE: When changing header row, update output (see below)
void USFLogObject::CreatePositionLogFile() {

	const FString& ParticipantInfix = USFGameInstance::Get()->GetParticipant()->GetParticipantLoggingInfix();
	ILogStream* PositionLog = UniLog.NewLogStream("PositionLog", "StudyFramework/StudyLogs/PositionLogs",
		"Position" + ParticipantInfix + ".txt", false);

	FString PositionLogHeader = FString("ElapsedTime") +
		"\t" + FString("LogName") +
		"\t" + FString("Condition") +
		"\t" + FString("Location-X") +
		"\t" + FString("Location-Y") +
		"\t" + FString("Location-Z") +
		"\t" + FString("Rotation-Pitch") +
		"\t" + FString("Rotation-Yaw") +
		"\t" + FString("Rotation-Roll") +
		"\t" + FString("Optional-Custom-Data");
	UniLog.Log(PositionLogHeader, "PositionLog");

	bPositionLoggingFileCreated = true;
}

void USFLogObject::WritePositionLogToFile() {
	if (!USFGameInstance::Get() || !USFGameInstance::Get()->GetLogObject() || !USFGameInstance::Get()->GetParticipant())
	{
		return;
	}

	USFLogObject* LogObject = USFGameInstance::Get()->GetLogObject();

	if(!USFGameInstance::Get()->GetLogObject()->bPositionLoggingFileCreated && LogObject->ComponentLoggingInfoArray.Num()>0)
	{
		USFGameInstance::Get()->GetLogObject()->CreatePositionLogFile();
	}
	
	for (auto& ComponentLoggingInfo : LogObject->ComponentLoggingInfoArray) {
		if (ComponentLoggingInfo.LogNextTick == true) {
			ComponentLoggingInfo.LogNextTick = false;
			//When starting in Debug-Mode (i.e. not through the HUD) no condition is defined. 
			FString CurrentCondition = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition() ?
				USFGameInstance::Get()->GetParticipant()->GetCurrentCondition()->UniqueName :
				FString("Debug");
			// NOTE: When changing output, update header row (see above)

			FString out = USFGameInstance::Get()->GetParticipant()->GetCurrentTimeAsString() +
				"\t" + ComponentLoggingInfo.LogName +
				"\t" + CurrentCondition +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentLocation().X) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentLocation().Y) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentLocation().Z) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentRotation().Pitch) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentRotation().Yaw) +
				"\t" + FString::Printf(TEXT("%.3f"), ComponentLoggingInfo.ComponentToLog->GetComponentRotation().Roll);
			USFLogCustomDataComponent* CustomDataComp = Cast<USFLogCustomDataComponent>(ComponentLoggingInfo.ComponentToLog);
			if(CustomDataComp)
			{
				out += "\t" + CustomDataComp->CustomData;
			}
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
void USFLogObject::CreateGazeTrackingLogFile() {

	const FString& ParticipantInfix = USFGameInstance::Get()->GetParticipant()->GetParticipantLoggingInfix();
	ILogStream* GazeTrackingLog = UniLog.NewLogStream("GazeTrackingLog", "StudyFramework/StudyLogs/GazeTrackingLogs",
		"GazeTracking" + ParticipantInfix + ".txt", false);

	FString GazeTrackingLogHeader = FString("ElapsedTime") +
		"\t" + FString("Condition") +
		"\t" + FString("TrackingEyes") +
		"\t" + FString("GazeTarget") +
		"\t" + FString("Gaze-Origin-X-Y-Z") +
		"\t" + FString("Gaze-Direction-X-Y-Z") +
		"\t" + FString("EyeOpenness") +
		"\t" + FString("PupilDiameterInMm");
	UniLog.Log(GazeTrackingLogHeader, "GazeTrackingLog");

	bGazingLoggingFileCreated = true;
}

void USFLogObject::WriteGazeTrackingLogToFile() {
	if (!USFGameInstance::Get())
	{
		return;
	}

	USFGazeTracker* GazeTracker = USFGameInstance::Get()->GetGazeTracker();
	if(!GazeTracker)
	{
		return;
	}

	if (!USFGameInstance::Get()->GetLogObject()->bGazingLoggingFileCreated)
	{
		USFGameInstance::Get()->GetLogObject()->CreateGazeTrackingLogFile();
	}

	FString GazeTarget = GazeTracker->GetCurrentGazeTarget();
	GazeTarget = (GazeTarget == "" ? "-" : GazeTarget);
	//When starting in Debug-Mode (i.e. not through the HUD) no condition is defined. 
	FString CurrentCondition = USFGameInstance::Get()->GetParticipant()->GetCurrentCondition() ?
								USFGameInstance::Get()->GetParticipant()->GetCurrentCondition()->UniqueName :
								FString("Debug");
	FString isTrackingEyes = USFGameInstance::Get()->GetGazeTracker()->IsTrackingEyes() ? "1" : "0";
	const float EyeOpenness = USFGameInstance::Get()->GetGazeTracker()->GetEyesOpenness();
	const float PupilDiameter = USFGameInstance::Get()->GetGazeTracker()->GetPupilDiameter();
	// NOTE: When changing output, update header row (see above)
	FString out = USFGameInstance::Get()->GetParticipant()->GetCurrentTimeAsString() +
		"\t" + CurrentCondition +
		"\t" + isTrackingEyes +
		"\t" + GazeTarget +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Origin.X) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Origin.Y) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Origin.Z) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Direction.X) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Direction.Y) +
		"\t" + FString::Printf(TEXT("%.3f"), GazeTracker->GetWorldGazeDirection().Direction.Z) +
		"\t" + (EyeOpenness < 0.0f ? "-" : FString::SanitizeFloat(EyeOpenness)) +
		"\t" + (PupilDiameter <= 0.0f ? "-" : FString::SanitizeFloat(PupilDiameter));
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
