using UnrealBuildTool;

public class StepVrDataStreamEditor : ModuleRules
{
	public StepVrDataStreamEditor(ReadOnlyTargetRules Target) : base(Target)
    {

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd", "StepVrDataStreamCore",  "AnimGraph", "BlueprintGraph" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "MessageLog"});
    }
}