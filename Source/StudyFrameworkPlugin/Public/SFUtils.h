// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"

class FJsonObject;

DECLARE_LOG_CATEGORY_EXTERN(SFLog, Log, All);


class FSFUtils
{
public:

	// Open Message Box with Text. bool error expresses whether to use excl. marks ++ //
	static void OpenMessageBox(FString Text, bool bError);
	static FString AddExclamationMarkAroundChar(FString Text);

	// Log into VALog
	static void LogStuff(FString Text, bool Error);

	// Print to Screen
	static void PrintToScreen(FString Text, float Time = 5.0f, FColor Color = FColor::Red);

    template <class T>
    static void CheckLoggingDataType(T Data);


    static FString SetupToString(TArray<int> Setup);


    static FString JsonToString(TSharedPtr<FJsonObject> Json);

    static TSharedPtr<FJsonObject> StringToJson(FString String);

private:
	static int KeyCounter;

};


// TODO is this nec?
template <class T>
void FSFUtils::CheckLoggingDataType(T Data)
{
    static_assert(
        std::is_same<T, float>::value ||
        std::is_same<T, bool>::value ||
        std::is_same<T, FString>::value ||
        std::is_same<T, int>::value ||
        std::is_same<T, TArray<float>>::value ||
        std::is_same<T, TArray<bool>>::value ||
        std::is_same<T, TArray<FString>>::value ||
        std::is_same<T, TArray<int>>::value,
        "Not correct Type!!");
}
