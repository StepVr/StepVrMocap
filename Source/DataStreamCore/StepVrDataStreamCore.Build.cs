using UnrealBuildTool;
using System;
using System.IO;


public class StepVrDataStreamCore : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }
    private string LibPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty")); }
    }

    public string GetLibFullPath()
    {
        string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x32";
        string LibrariesPath = Path.Combine(LibPath, "lib", PlatformString);
        return LibrariesPath;
    }

    public void ReferenceDlls()
    {
        Console.WriteLine("-------------StepVrMocap Start-------------");

        string Path = GetLibFullPath();
        var DllFiles = Directory.GetFiles(Path, "*.dll");
        foreach (var file in DllFiles)
        {
            RuntimeDependencies.Add(file);
            Console.WriteLine(file);
        }

        var SktNameFiles = Directory.GetFiles(LibPath + "/skt", "*.txt");
        foreach (var file in SktNameFiles)
        {
            RuntimeDependencies.Add(file);
            Console.WriteLine(file);
        }

        var SktFiles = Directory.GetFiles(LibPath + "/skt", "*.skt");
        foreach (var file in SktFiles)
        {
            RuntimeDependencies.Add(file);
            Console.WriteLine(file);
        }

        Console.WriteLine("-------------StepVrMocap End-------------");
    }

    public StepVrDataStreamCore(ReadOnlyTargetRules Target) : base(Target)
    {
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //初始化
        InitStepMagic(false);

        //加载StepVrPlugins


        PrivateIncludePaths.AddRange( new string[] 
        {
            "DataStreamCore/Private",
            "DataStreamCore/Public",
        } );


        PrivateDependencyModuleNames.AddRange(new string[] {
            "Projects",
            "Json",
            "JsonUtilities"
        });
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "Sockets"
        });

        //加载DLL
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateIncludePaths.Add(Path.Combine(LibPath, "include"));
            PublicIncludePaths.Add(Path.Combine(LibPath, "include"));

            string LibrariesPath = GetLibFullPath();

            PublicDelayLoadDLLs.Add("StepIKClientDllCPP.dll");
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "StepIKClientDllCPP.lib"));

            ReferenceDlls();
        }
    }

    public void InitStepMagic(bool IsValid)
    {
        if (Target.Platform == UnrealTargetPlatform.Win64 && IsValid)
        {
            PublicDefinitions.Add("WITH_STEPMAGIC");
            PrivateDependencyModuleNames.Add("AJA_Corvid44");
        }
    }
}
