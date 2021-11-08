// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SFPlugin.h"


#include "IDisplayCluster.h"						  // For bIsMaster
#include "Cluster/IDisplayClusterClusterManager.h"	  // For bIsMaster


bool FSFPlugin::bIsMaster = false;

#define LOCTEXT_NAMESPACE "FStudyFrameworkPluginModule"

void FSFPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	bIsMaster = IDisplayCluster::Get().GetOperationMode() != EDisplayClusterOperationMode::Cluster || IDisplayCluster::
		Get().GetClusterMgr()->IsMaster();
}

void FSFPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

bool FSFPlugin::GetIsMaster()
{
	return bIsMaster;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSFPlugin, StudyFrameworkPlugin)
