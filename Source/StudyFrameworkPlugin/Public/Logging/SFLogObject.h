// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SFLogObject.generated.h"

USTRUCT(BlueprintType)
struct FActorLoggingInformation {
	GENERATED_BODY()
public:
	FActorLoggingInformation() {}

	FActorLoggingInformation(int32 LogTimer, AActor* Actor, FString LogName) {
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
	UPROPERTY()
		FString LogName;
	UPROPERTY()
		FDateTime TimeStorage;
};

UCLASS()
class StudyFrameworkPlugin_API USFLogObject : public UObject {
	GENERATED_BODY()

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
