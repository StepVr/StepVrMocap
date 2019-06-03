﻿// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#include "StepMocapDefine.h"

#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Json.h"




DEFINE_LOG_CATEGORY(LogStepMocap)

TArray<FString> StepFaceMorphTargets = {};

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

void LoadMorphTargets()
{
	FString JsonData;
	if (!FFileHelper::LoadFileToString(JsonData, TEXT("C://StepFace//config//configBlends.json")))
	{
		ShowMessage(TEXT("Read C://StepFace//config//configBlends.json Fail"));
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
		ShowMessage(TEXT("Parse MorphTargets Config Error"));
	}
}