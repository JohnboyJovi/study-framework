// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SFParticipant.h"
#include "SFDefines.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFManagerActor.generated.h"


UCLASS()
class STUDYFRAMEWORKPLUGIN_API ASFManagerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASFManagerActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void StartStudy();

	UFUNCTION(BlueprintCallable)
	void NextSetup();

	
	

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	SFParticipant* CurrentParticipant;

	TArray<FStudySetting> Settings;

};
