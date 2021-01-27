// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


// FStudyFrameworkPluginModule

// TODO Check .cs file for correct Dependencies

class FSFPlugin : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	
	static bool GetIsMaster();


protected:

	static bool bIsMaster;
};
