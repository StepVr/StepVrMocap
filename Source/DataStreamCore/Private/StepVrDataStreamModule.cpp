#include "StepVrDataStreamModule.h"
#include "FaceExc/StepVrFaceSourceUDP.h"
#include "FaceExc/StepVrFaceSource.h"
#include "StepMocapDefine.h"
#include "StepVrSkt.h"


#include "Engine.h"



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
	//加载动捕skt
	STEPVRSKT->LoadSkt();

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


	//ARKit
	FStepListenerToAppleARKit::CreateRemoteListener();
	FStepListenerToSelf::CreateRemoteListener();
}


void FStepDataStreamModule::ShutdownModule()
{
	FreeHandle();
}

FStepListenerToAppleARKit* FStepDataStreamModule::GetStepListenerToAppleARKit()
{
	return FStepListenerToAppleARKit::CreateRemoteListener();
}

FStepListenerToSelf* FStepDataStreamModule::GetStepListenerToSelf()
{
	return FStepListenerToSelf::CreateRemoteListener();
}

IMPLEMENT_MODULE(FStepDataStreamModule, StepVrDataStreamCore);