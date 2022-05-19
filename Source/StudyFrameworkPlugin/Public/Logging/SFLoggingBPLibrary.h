// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SFLoggingBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS()
class USFLoggingBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Log To File", Keywords = "SFLogging"), Category = "SFLogging")
		static void LogToFile();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Log To One File", Keywords = "SFLogging"), Category = "SFLogging")
		static void LogToOneFile(int32 Timer);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Actor To Log", Keywords = "SFLogging"), Category = "SFLogging")
		static void AddActorToLog(AActor* Actor, int32 LogTimer);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Actor To Log With Name", Keywords = "SFLogging"), Category = "SFLogging")
		static void AddActorToLogWithName(AActor* Actor, int32 LogTimer, FString LogName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Actors To Log", Keywords = "SFLogging"), Category = "SFLogging")
		static void AddActorsToLog(TArray<AActor*> ActorArray, int32 LogTimer);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize", Keywords = "SFLogging"), Category = "SFLogging")
		static void Initialize();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Log Custom Event", Keywords = "SFLogging"), Category = "SFLogging")
		static void LogCustomEvent(FString EventName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Log Custom Event with Actor", Keywords = "SFLogging"), Category = "SFLogging")
		static void LogCustomEventWithActor(FString EventName, AActor* Actor);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Log Custom Event with Actor and Name", Keywords = "SFLogging"), Category = "SFLogging")
		static void LogCustomEventWithActorAndName(FString EventName, AActor* Actor, FString ActorName);
};
