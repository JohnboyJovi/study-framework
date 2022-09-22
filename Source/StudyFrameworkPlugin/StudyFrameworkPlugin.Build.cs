// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class StudyFrameworkPlugin : ModuleRules
{
	public StudyFrameworkPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				// "Core",
                // "DisplayCluster",
                // "DisplayClusterExtensions",
                // TODO Check what is necessary
                "Core",
                "Projects",
                "CoreUObject",
                "Engine",
                "DisplayCluster",
                "RWTHVRToolkit",
                "RWTHVRCluster",
                "InputCore",
                "Sockets",
                "Json",
                "JsonUtilities",
                "Networking",
                "UMG",
                "HeadMountedDisplay",
                "SlateCore"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "UniversalLogging",
				// ... add private dependencies that you statically link with here ...	
			}
			);
			
		//this is needed to register on Editor delegates, i.e., BeginPIE and EndPIE, but only in Editor builds
        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }
		
        //check for SRanipal which we would need if eyetracking should be used
        //see https://devhub.vr.rwth-aachen.de/VR-Group/unreal-development/plugins/unreal-study-framework/-/wikis/EyeTracking how to add it
        string PluginsPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../.."));
        bool bSRanipalPlugin = Directory.Exists(Path.Combine(PluginsPath, "SRanipal"));
        if(bSRanipalPlugin)
        {
            PrivateDefinitions.Add("WITH_SRANIPAL");
            PrivateDependencyModuleNames.AddRange(new string[] {"SRanipal", "SRanipalEye" });
        }
        
	}
}
