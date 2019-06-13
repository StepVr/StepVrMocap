// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.

#include "StepVrDataStreamModule.h"
#include "Engine.h"
#include "StepMocapDefine.h"



static void* CPPUdpClientDllHandle = nullptr;

void FreeHandle()
{
	if (CPPUdpClientDllHandle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(CPPUdpClientDllHandle);
		CPPUdpClientDllHandle = nullptr;
	}
}

void FStepDataStreamModule::StartupModule()
{
	//加载面捕数据
	LoadMorphTargets();

	TArray<FString> DllPaths;
	FString Platform = (PLATFORM_WINDOWS&&PLATFORM_64BITS) ? TEXT("x64") : TEXT("x32");
	DllPaths.Add(FPaths::ProjectPluginsDir() + TEXT("StepVrMocap/ThirdParty/lib/") + Platform);
	DllPaths.Add(FPaths::EnginePluginsDir() + TEXT("StepVrMocap/ThirdParty/lib/") + Platform);
	DllPaths.Add(FPaths::EnginePluginsDir() + TEXT("Runtime/StepVrMocap/ThirdParty/lib/") + Platform);

	for (int32 i = 0; i < DllPaths.Num(); i++)
	{
		FPlatformProcess::PushDllDirectory(*DllPaths[i]);
		
		//加载动捕DLL
		CPPUdpClientDllHandle = FPlatformProcess::GetDllHandle(*(DllPaths[i] + "/StepIKClientDllCPP.dll"));

		FPlatformProcess::PopDllDirectory(*DllPaths[i]);

		if (CPPUdpClientDllHandle != nullptr)
		{
			break;
		}
	}

	if (CPPUdpClientDllHandle == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Load StepMocapDll Faild"));
	}
}


void FStepDataStreamModule::ShutdownModule()
{
	FreeHandle();
}

IMPLEMENT_MODULE(FStepDataStreamModule, StepVrDataStreamCore);