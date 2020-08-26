// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#include "StepMocapDefine.h"

#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Json.h"




DEFINE_LOG_CATEGORY(LogStepVrStreamer)

TArray<FString> StepFaceMorphTargets = {};

void StepMocapSpace::ShowMessage(const FString& Log)
{
	if (IsInGameThread())
	{
		UE_LOG(LogStepVrStreamer, Log, TEXT("%s"), *Log);
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [Log]()
		{
			StepMocapSpace::ShowMessage(Log);
			//UE_LOG(LogStepMocap, Log, TEXT("%s"), *Log);
		});
	}
}

FString StepMocapSpace::GetLocalIP()
{
	static FString GLocalIP = "";
	if (GLocalIP.IsEmpty())
	{
		bool CanBind = false;
		auto OnlinePtr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (OnlinePtr)
		{
			TSharedRef<FInternetAddr> LocalIp = OnlinePtr->GetLocalHostAddr(*GLog, CanBind);
			if (LocalIp->IsValid())
			{
				GLocalIP = LocalIp->ToString(false);
			}
		}
	}
	return GLocalIP;
}

FString StepMocapSpace::Convert2LocalIP(const FString& NewIP)
{
	FString Addr = NewIP;
	if (NewIP.Equals(TEXT("127.0.0.1"), ESearchCase::IgnoreCase) ||
		NewIP.Equals(TEXT("localhost"), ESearchCase::IgnoreCase))
	{
		Addr = StepMocapSpace::GetLocalIP();
	}

	return Addr;
}

bool StepMocapSpace::IsLocalIP(const FString& CheckIP)
{
	if (CheckIP.Equals(TEXT("127.0.0.1"), ESearchCase::IgnoreCase) ||
		CheckIP.Equals(GetLocalIP(), ESearchCase::IgnoreCase) ||
		CheckIP.Equals(TEXT("localhost"), ESearchCase::IgnoreCase))
	{
		return true;
	}

	return false;
}

void LoadMorphTargets()
{
	FString JsonData;
	if (!FFileHelper::LoadFileToString(JsonData, TEXT("C://StepFace//config//configBlends.json")))
	{
		StepMocapSpace::ShowMessage(TEXT("Read C://StepFace//config//configBlends.json Fail"));
		return;
	}

	bool IsSuccess = false;
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonData);
	do 
	{
		if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
			break;
		}
		if (!JsonObject.IsValid())
		{
			break;
		}
		TSharedPtr<FJsonObject> JsonObject1 = JsonObject->GetObjectField(TEXT("ValidBlendShapes"));
		if (!JsonObject1.IsValid())
		{
			break;
		}

		for (auto& Temp : JsonObject1->Values)
		{
			StepFaceMorphTargets.Add(Temp.Key);
		}
		IsSuccess = true;
	} while (0);

	if (!IsSuccess)
	{
		StepMocapSpace::ShowMessage(TEXT("Parse MorphTargets Config Error"));
	}
}
