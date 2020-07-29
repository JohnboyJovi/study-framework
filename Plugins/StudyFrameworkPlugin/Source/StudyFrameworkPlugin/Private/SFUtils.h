// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"

DECLARE_LOG_CATEGORY_EXTERN(SFLog, Log, All);


class FSFUtils
{
public:

	// Open Message Box with Text. bool error expresses whether to use excl. marks ++ //
	static void OpenMessageBox(FString Text, bool bError);
	static FString AddExclamationMarkAroundChar(FString Text);

	// Log into VALog
	static void LogStuff(FString Text, bool Error = false);

};
