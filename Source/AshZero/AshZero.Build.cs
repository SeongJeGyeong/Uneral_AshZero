// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class AshZero : ModuleRules
{
	public AshZero(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] { "AshZero" });
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule", "EnhancedInput", 
			"UMG", "Niagara", "NavigationSystem", "AIModule", "GameplayTags", "OnlineSubsystem", "OnlineSubsystemUtils", 
            "MoviePlayer", "Slate", "SlateCore", "MediaAssets", "PhysicsCore", "LevelSequence", "MovieScene"});

        PrivateDependencyModuleNames.AddRange(new string[] { "AdvancedSessions" });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
