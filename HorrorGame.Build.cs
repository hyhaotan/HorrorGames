// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HorrorGame : ModuleRules
{
    public HorrorGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {   "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "AIModule",
            "GameplayTasks",
            "CommonUI",
            "NavigationSystem",
            "Paper2D",
            "HeadMountedDisplay",
            "EnhancedInput",
            "UMG",
            "SlateCore",
            "Slate",
            "Niagara",
            "GameplayCameras",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "OnlineSubsystemSteam"});

        //DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
    }
}
