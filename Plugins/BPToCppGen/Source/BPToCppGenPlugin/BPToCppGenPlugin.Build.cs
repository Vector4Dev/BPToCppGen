using UnrealBuildTool;

public class BPToCppGenPlugin : ModuleRules
{
    public BPToCppGenPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "EditorStyle",
            "UnrealEd",
            "WorkspaceMenuStructure",
            "ToolMenus",
            "DesktopPlatform",
            "InputCore",
            "BlueprintGraph",
            "Kismet",
            "ContentBrowser",
            "AssetRegistry"
        });
    }
}