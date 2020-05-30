using UnrealBuildTool;

public class StepVrDataStreamEditor : ModuleRules
{
	public StepVrDataStreamEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(new string[]
        {
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "AnimationCore",
            "AnimGraphRuntime",
            "Core",
            "CoreUObject",
            "Engine",

            "StepVrDataStreamCore",

            "Slate", 
            "MessageLog",
        });


        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                      "UnrealEd",
                      "Kismet",
                      "AnimGraph",
                      "BlueprintGraph",
                }
            );
        }

    }
}