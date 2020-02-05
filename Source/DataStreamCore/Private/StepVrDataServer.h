#pragma once
#include "CoreMinimal.h"


DECLARE_DELEGATE(FReceiveData)

class StepVrDataServer
{
public:
	StepVrDataServer();
	virtual ~StepVrDataServer();

	//创建链接
	static TSharedPtr<StepVrDataServer> CreateServerData();
	
	//还原skt
	virtual void ReplaceSkt(bool IsUse);

	//数据回调
	FReceiveData ReceiveData;

	//链接
	virtual void Connect2Server(const FString& IP,int32 port);
	virtual void DisConnect();
	virtual bool IsConnected() = 0;

	//动捕数据
	virtual bool HasBodyData() = 0;
	virtual void GetBodyData(TArray<FTransform>& OutData) = 0;

	//手套数据
	virtual bool HasHandData() = 0;
	virtual void GetHandData(TArray<FRotator>& OutData) = 0;

	//面部数据
	virtual bool HasFaceData() = 0;
	virtual void GetFaceData(TMap<FString, float>& OutData) = 0;

protected:
	FString ServerIP;
	int32   ServerPort;
};