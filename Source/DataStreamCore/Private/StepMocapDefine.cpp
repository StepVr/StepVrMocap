// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#include "StepMocapDefine.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"

DEFINE_LOG_CATEGORY(LogStepMocap)

void ShowMessage(const FString& Log)
{
	if (IsInGameThread())
	{
		UE_LOG(LogStepMocap, Log, TEXT("%s"), *Log);
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [Log]()
		{
			ShowMessage(Log);
			//UE_LOG(LogStepMocap, Log, TEXT("%s"), *Log);
		});
	}
}

FString GetLocalIP()
{
	bool CanBind = false;
	FString IPAddress = "";
	TSharedRef<FInternetAddr> LocalIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, CanBind);
	if (LocalIp->IsValid())
	{
		IPAddress = LocalIp->ToString(false);
	}

	return IPAddress;
}

FString Convert2LocalIP(const FString& NewIP)
{
	FString Addr = NewIP;
	if (NewIP.Equals(TEXT("127.0.0.1"), ESearchCase::IgnoreCase) ||
		NewIP.Equals(TEXT("localhost"), ESearchCase::IgnoreCase))
	{
		Addr = GetLocalIP();
	}

	return Addr;
}
