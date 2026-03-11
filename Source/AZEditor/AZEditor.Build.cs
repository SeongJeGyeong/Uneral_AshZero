using UnrealBuildTool;

public class AZEditor: ModuleRules
{
    public AZEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicIncludePaths.AddRange(new string[] { "AZEditor" });

        PrivateIncludePaths.AddRange(new string[]
        {
            System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Editor/Blutility/Private"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", 
                "Slate", "SlateCore", "UnrealEd", "EditorStyle",
                "ToolMenus", "PropertyEditor", "GameplayTagsEditor",
                "DataTableEditor", "InputCore", "GameplayTags",
                "Blutility", "UMGEditor"
        });

        // AZEditor 모듈에서 AshZero 모듈의 클래스를 include하기 위해 필요 
        PublicDependencyModuleNames.AddRange(new string[] {"AshZero"} );
    }
}
