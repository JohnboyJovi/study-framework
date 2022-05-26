#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SFLogObject.generated.h"

USTRUCT(BlueprintType)
struct FActorLoggingInformation
{
	GENERATED_BODY()
public:
	FActorLoggingInformation()
	{
	}

	FActorLoggingInformation(int32 LogTimer, AActor* Actor, FString LogName)
	{
		this->LogTimer = LogTimer;
		this->LogNextTick = false;
		this->ActorToLog = Actor;
		this->LogName = LogName;
		TimeStorage = FDateTime::Now();
	}

	UPROPERTY()
	int32 LogTimer;
	UPROPERTY()
	bool LogNextTick;
	UPROPERTY()
	AActor* ActorToLog;
	// eventually, other UObjects like components should be able to be tracked as well,
	// so this should be a UObject instead. Or the function needs to be overloaded to allow dif object types.
	UPROPERTY()
	FString LogName;
	UPROPERTY()
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
	TArray<FActorLoggingInformation> LoggingInfo;
	FDateTime StaticDateTime;
	FString LogDir = "";
	bool LogThis = false;
	int32 ProbandID;

	void AddActor(AActor* Actor, int32 LogTimer);
	void AddActorWithName(AActor* Actor, int32 LogTimer, FString LogName);
	void AddActors(TArray<AActor*>, int32 LogTimer);
	void Initialize();
	void SetUpLogDir();
};
