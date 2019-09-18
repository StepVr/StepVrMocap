#pragma once
#include "CoreMinimal.h"


class StepVrDataServer
{
public:
	StepVrDataServer();
	virtual ~StepVrDataServer();

	static TSharedPtr<StepVrDataServer> CreateServerData();

	//����
	virtual void Connect2Server(const FString& IP,int32 port) = 0;
	virtual void DisConnect() = 0;

	//��������
	virtual bool HasBodyData() = 0;
	virtual void GetBodyData(TArray<FTransform>& OutData) = 0;

	//��������
	virtual bool HasHandData() = 0;
	virtual void GetHandData(TArray<FRotator>& OutData) = 0;

	//�沿����
	virtual bool HasFaceData() = 0;
	virtual void GetFaceData(TMap<FString, float>& OutData) = 0;
};