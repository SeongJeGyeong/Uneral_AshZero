// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AshZeroEditorTarget : TargetRules
{
	public AshZeroEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;

        ExtraModuleNames.AddRange(new string[]
             {
            "AshZero",
            "AZEditor"
             });

        bUsesSteam = true;
        ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_STEAM=1");
    }
}
