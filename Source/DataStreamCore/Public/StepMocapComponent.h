// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StepMocapComponent.generated.h"


/*DECLARE_EVENT_OneParam*/


enum EOrderType
{
	Type_Tpose,
	Type_UpdateSkt,
	Type_NewIP,
	Type_NewFaceID,
	//面部缩放
	Type_ScaleFace,
	//获取骨骼缩放
	Type_GetSkeletonScale,
};


//控制数据流
UCLASS(ClassGroup = StepvrClassGroup, BlueprintType, meta = (BlueprintSpawnableComponent))
class STEPVRDATASTREAMCORE_API UStepMocapComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	~UStepMocapComponent();

	UFUNCTION(BlueprintCallable)
	void MocapUpdateSkt();

	UFUNCTION(BlueprintCallable)
	void MocapTPose();

	UFUNCTION(BlueprintCallable)
	void MocapSetNewIP(const FString& InNewIP);

	UFUNCTION(BlueprintCallable)
	void MocapSetNewFaceID(const FString& InNewFaceID);

	UFUNCTION(BlueprintCallable)
	void MocapSetNewFaceScale(float NewScale);

	/**
	 * 刷新后,获取骨骼缩放
	 * MocapGetSkeletonScale
	 */
	UFUNCTION(BlueprintCallable)
	void MocapRefreshSkeletonScale();
	UFUNCTION(BlueprintPure)
	void MocapGetSkeletonScale(FVector& Out);

	UFUNCTION(BlueprintCallable)
	void StartRecord(const FString& RecordName);
	UFUNCTION(BlueprintCallable)
	void StopRecord();
	UFUNCTION(BlueprintCallable)
	void PlayRecord(const FString& RecordName);
	UFUNCTION(BlueprintCallable)
	void GetAllRecords(TArray<FString>& RecordName);


	bool IsMocapReplicate();


	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	//网络同步
	UPROPERTY(BlueprintReadWrite)
	bool bMocapReplicate = false;

	FString NewServerIP = "";
	FString NewFaceID = "";

	//录制数据
	bool bRecord = false;
	FString strRecordName;

	//骨骼缩放
	FVector SkeletonScale = FVector::OneVector;

	//面部缩放
	float FaceScale = 1.f;

	//缓存指令
	TQueue<EOrderType> Orders;
};

