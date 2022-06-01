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
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 LogTimer;
	bool LogNextTick;
	// eventually, other UObjects like components should be able to be tracked as well,
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AActor* ActorToLog;
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
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(TitleProperty = "LogName"))
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
