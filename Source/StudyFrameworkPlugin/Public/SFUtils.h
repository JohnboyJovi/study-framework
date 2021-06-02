// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"

class FJsonObject;

class FSFUtils
{
public:

	// Open Message Box with Text. bool error expresses whether to use excl. marks ++ //
	static void OpenMessageBox(FString Text, bool bError = _CMP_FALSE_OS);

	// Log into Console and errors also on Screen, works also in Shipping build in contrast to UE_LOG
	static void Log(FString Text, bool Error = false);

	// To setup the debugging logs to be used
	static void SetupLoggingStreams();

    template <class T>
    static void CheckLoggingDataType(T Data);


    static FString SetupToString(TArray<int> Setup);


    static FString JsonToString(TSharedPtr<FJsonObject> Json);

    static TSharedPtr<FJsonObject> StringToJson(FString String);
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
