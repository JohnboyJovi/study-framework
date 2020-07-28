// Fill out your copyright notice in the Description page of Project Settings.


#include "SFManagerActor.h"

// Sets default values
ASFManagerActor::ASFManagerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASFManagerActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASFManagerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASFManagerActor::NextSetup()
{
	
}

void ASFManagerActor::StartStudy()
{
}
