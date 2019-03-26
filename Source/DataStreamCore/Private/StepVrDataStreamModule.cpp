// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.

#include "StepVrDataStreamModule.h"
#include "Engine.h"

static void* LocalDllHandle = nullptr;
static void* CPPUdpClientDllHandle = nullptr;
void FreeHandle()
{
	if (LocalDllHandle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(LocalDllHandle);
		LocalDllHandle = nullptr;
	}

	if (CPPUdpClientDllHandle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(CPPUdpClientDllHandle);
		CPPUdpClientDllHandle = nullptr;
	}
}

void FStepDataStreamModule::StartupModule()
{
	//动捕
	{
		if (LocalDllHandle)
		{
			FreeHandle();
		}

		FString Platform = (PLATFORM_WINDOWS&&PLATFORM_64BITS) ? TEXT("x64") : TEXT("x32");
		FString DllPath = FPaths::ProjectPluginsDir() + TEXT("StepVrMocap/ThirdParty/lib/") + Platform;

		FPlatformProcess::PushDllDirectory(*DllPath);
		LocalDllHandle = FPlatformProcess::GetDllHandle(*(DllPath + TEXT("/StepIKClientDllCPP.dll")));
		FPlatformProcess::PopDllDirectory(*DllPath);

		if (LocalDllHandle == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Load StepMocapDll Faild"));
		}
	}

	//新捕捉
	{
		if (CPPUdpClientDllHandle)
		{
			FreeHandle();
		}

		FString Platform = (PLATFORM_WINDOWS&&PLATFORM_64BITS) ? TEXT("x64") : TEXT("x32");
		FString DllPath = FPaths::ProjectPluginsDir() + TEXT("StepVrMocap/ThirdParty/lib/") + Platform;

		FPlatformProcess::PushDllDirectory(*DllPath);
		CPPUdpClientDllHandle = FPlatformProcess::GetDllHandle(*(DllPath + TEXT("/CPPUdpClient.dll")));
		FPlatformProcess::PopDllDirectory(*DllPath);

		if (LocalDllHandle == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Load CPPUdpClientDLL Faild"));
		}
	}
}


void FStepDataStreamModule::ShutdownModule()
{
	FreeHandle();
}

IMPLEMENT_MODULE(FStepDataStreamModule, StepVrDataStreamCore);