// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ARTrackable.h"
#include "StepMocapComponent.generated.h"


/*DECLARE_EVENT_OneParam*/


enum EOrderType
{
	Type_EnableStream,
	Type_EnableSkeletonScale,
	Type_Tpose,
	Type_UpdateSkt,
	Type_NewIP,
	Type_NewFaceID,
	//面部缩放
	Type_ScaleFace,
	//开启关闭面部点
	Type_EnableFace,
	//切换使用面部数据类型
	Type_ChangeFace,
	//获取骨骼缩放
	Type_GetSkeletonScale,
};


UENUM(BlueprintType)
enum class EUseFaceTypeEx : uint8
{
	Face_None,
	Face_Iphone,
	Face_Voice,
};



//控制数据流
UCLASS(ClassGroup = StepvrClassGroup, BlueprintType, meta = (BlueprintSpawnableComponent))
class STEPVRDATASTREAMCORE_API UStepMocapComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	~UStepMocapComponent();

	//关闭/开启动捕数据流
	UFUNCTION(BlueprintCallable)
	void MocapStopSkeletonCapture(bool bStop);

	//切换服务SKT
	UFUNCTION(BlueprintCallable)
	void MocapUpdateSkt();

	//动捕TPOSE，进行校准
	UFUNCTION(BlueprintCallable)
	void MocapTPose();

	//切换动捕数据流
	UFUNCTION(BlueprintCallable)
	void MocapSetNewIP(const FString& InNewIP);

	//切换面部数据流
	UFUNCTION(BlueprintCallable)
	void MocapSetNewFaceID(const FString& InNewFaceID);

	//修改面部数据缩放，控制表情幅度
	UFUNCTION(BlueprintCallable)
	void MocapSetNewFaceScale(float NewScale);

	//关闭某个面部点数据
	UFUNCTION(BlueprintCallable)
	void MocapSetEnableFace(bool Enable , EARFaceBlendShape ARFaceBlendShape);


	//关闭某个面部点数据
	UFUNCTION(BlueprintCallable)
	void MocapChangeFaceType(EUseFaceTypeEx UseFaceType);

	/**
	 * 刷新后,获取骨骼缩放,需要Delay后再调用MocapGetSkeletonScale
	 */
	UFUNCTION(BlueprintCallable)
	void MocapRefreshSkeletonScale();
	UFUNCTION(BlueprintCallable)
	void MocapSetScaleEnable(bool NewState);
	UFUNCTION(BlueprintPure)
	void MocapGetSkeletonScale(FVector& Out);

	//服务 开始/结束 录制数据
	UFUNCTION(BlueprintCallable)
	void StartRecord(const FString& RecordName);
	UFUNCTION(BlueprintCallable)
	void StopRecord();

	//服务播放回放数据
	UFUNCTION(BlueprintCallable)
	void PlayRecord(const FString& RecordName);
	UFUNCTION(BlueprintCallable)
	void GetAllRecords(TArray<FString>& RecordName);


	bool IsMocapReplicate();

protected:
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//网络同步
	UPROPERTY(BlueprintReadWrite)
	bool bMocapReplicate = false;

	FString NewServerIP = "";
	FString NewFaceID = "";

	//是否开启数据流
	bool EnableStream = false;
	bool EnableSkeletonScale = false;

	//录制数据
	bool bRecord = false;
	FString strRecordName;

	//骨骼缩放
	FVector SkeletonScale = FVector::OneVector;

	//切换面部
	EUseFaceTypeEx UseFaceTypeEx;

	//面部缩放
	float FaceScale = 1.f;

	//面部点
	TMap<EARFaceBlendShape , bool> NewFaceState;

	//缓存指令
	TQueue<EOrderType> Orders;
};

