#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SFLogObject.generated.h"

USTRUCT(BlueprintType)
struct FComponentLoggingInformation
{
	GENERATED_BODY()
public:
	FComponentLoggingInformation()
	{
	}

	FComponentLoggingInformation(int32 LogTimer, USceneComponent* Component, FString LogName)
	{
		this->LogTimer = LogTimer;
		this->LogNextTick = false;
		this->ComponentToLog = Component;
		this->LogName = LogName;
		TimeStorage = FDateTime::Now();
	}
	// Specifies Logging Frequency in ms
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		int32 LogTimer;
	bool LogNextTick;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		USceneComponent* ComponentToLog;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		FString LogName;
	FDateTime TimeStorage;
};

UCLASS()
class STUDYFRAMEWORKPLUGIN_API USFLogObject : public UObject
{
	GENERATED_BODY()
public:
	friend class SFPlugin;

private:
	USFLogObject();

public:
	void AddComponentWithName(USceneComponent* Component, int32 LogTimer, FString LogName);

	void RemoveEntryByComponent(const USceneComponent* Component);
	void RemoveAllTrackedComponents();
	FComponentLoggingInformation* GetEntryByComponent(const USceneComponent* Component);
	void Initialize();

	//NOTE: This is for internal use, it is called within FadeHandler
	// when starting FadeOut / finished FadeIn.
	// it will not disable logging permanently if set to false
	UFUNCTION()
	void SetLoggingLoopsActive(bool LoggingLoopsActive);

	UFUNCTION()
	bool GetLoggingLoopsActive();

	UFUNCTION()
	static void WritePositionLogToFile();

	UFUNCTION()
	static void WriteGazeTrackingLogToFile();

private:

	void CreatePositionLogFile();
	void CreateGazeTrackingLogFile();

	TArray<FComponentLoggingInformation> ComponentLoggingInfoArray;
	FDateTime StaticDateTime;
	bool bLoggingLoopsActive = false;

	bool bPositionLoggingFileCreated = false;
	bool bGazingLoggingFileCreated = false;
};
