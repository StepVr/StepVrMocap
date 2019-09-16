#pragma once
#include "CoreMinimal.h"




class StepVrDataServer
{
public:
	StepVrDataServer();
	virtual ~StepVrDataServer();

	virtual bool HasBodyData() = 0;
	virtual bool HasHandData() = 0;
	virtual bool HasFaceData() = 0;

	//����
	virtual void Connect2Server(const FString& IP,int32 port) = 0;
	//�Ͽ�����
	virtual void DisConnect() = 0;
	//��������
	virtual void GetBodyData(TArray<FTransform>& OutData) = 0;
	//��������
	virtual void GetHandData(TArray<FRotator>& OutData) = 0;
	//�沿����
	virtual void GetFaceData(TMap<FString, float>& OutData) = 0;
};

class FServicesData : public StepVrDataServer
{
public:

	virtual bool HasBodyData() override;


	virtual bool HasHandData() override;


	virtual bool HasFaceData() override;


	virtual void Connect2Server(const FString& IP, int32 port) override;


	virtual void DisConnect() override;


	virtual void GetBodyData(TArray<FTransform>& OutData) override;


	virtual void GetHandData(TArray<FRotator>& OutData) override;


	virtual void GetFaceData(TMap<FString, float>& OutData) override;
};


