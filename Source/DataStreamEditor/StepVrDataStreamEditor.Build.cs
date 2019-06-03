using UnrealBuildTool;

public class StepVrDataStreamEditor : ModuleRules
{
	public StepVrDataStreamEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(new string[]
        {
            "DataStreamEditor/Private",
            "DataStreamEditor/Public",
            "DataStreamEditor/Classes",
        });

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd", "StepVrDataStreamCore",  "AnimGraph", "BlueprintGraph" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "MessageLog"});
    }
}