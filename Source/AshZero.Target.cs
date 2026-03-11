// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AshZeroTarget : TargetRules
{
	public AshZeroTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("AshZero");

        bUsesSteam = true;
        ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_STEAM=1");
    }
}
