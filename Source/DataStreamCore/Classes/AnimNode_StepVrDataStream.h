// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.


#pragma once

#include "StepVrStream.h"
#include "AnimNode_StepVrDataStream.generated.h"

enum FStepControllState
{
	Invalid,
	Local,
	Remote,
};


USTRUCT(BlueprintType)
struct  STEPVRDATASTREAMCORE_API FAnimNode_StepDataStream : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

	// The input pose is segmented into two:
	// - The FK input pose that serves as a bias for the solver.
	// - The task targets appended at the end.
	FPoseLink InPose;
	
	/**
	 * Step默认骨骼匹配UE4使用骨骼
	 * 修改对应的骨骼，不要增删骨骼
	 */
	UPROPERTY(EditAnywhere, Category = StepMocapBindBones)
	TMap<FString, FBoneReference>	BindMocapBones;

	UPROPERTY(EditAnywhere, Category = StepMocapBindBones)
	TMap<FString, FBoneReference>	BindMocapHandBones;

	/**
	 * 需要归零多余的骨骼
	 */
	UPROPERTY(EditAnywhere, Category = StepMocapBindBones)
	TArray<FBoneReference>	RedundanceMocapBones;

	UPROPERTY(EditAnywhere, Category=Server, meta=(PinShownByDefault))
	FName ServerName = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, Category=Server,meta=(PinShownByDefault))
	int32 PortNumber = 9516;

	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault, ToolTip = "Apply character scale from Step"))
	bool ApplyStepScale = false;

public:	

	FAnimNode_StepDataStream();
	~FAnimNode_StepDataStream();

	void BindSkeleton(FAnimInstanceProxy* AnimInstanceProxy);
	void BindServerStream(FAnimInstanceProxy* AnimInstanceProxy);
	void IntializeServerStreamer(FAnimInstanceProxy* AnimInstanceProxy);
	
	// FAnimNode_Base interface
	void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	void Evaluate_AnyThread(FPoseContext& Output) override;
	void CacheBones_AnyThread(const FAnimationCacheBonesContext & Context) override;
	// End of FAnimNode_Base interface
	
	virtual void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;

private:
	void InitData();

	FString CheckConert2LocalIP(const FString& IP);

	//绑定骨架
	FStepDataToSkeletonBinding mSkeletonBinding;
	
	//骨骼姿态
	TArray<FTransform> BonesData;
	//多余的骨骼号
	TArray<int32> RedundanceMocapBoneID;

	//手部骨骼姿态
	TArray<FRotator> HandBonesData;

	bool DataIsReady = false;

	//链接server属性
	FMocapServerInfo MocapServerInfo;

	const FAnimInstanceProxy* CacheAnimInstanceProxy = nullptr;
	FStepControllState StepControllState = FStepControllState::Invalid;
	const APawn* OwnerPawn = nullptr;
	uint32 PlayerAddrID = 0;
	bool IsInit = false;

// 	IKinemaReplicateData	FrameCacheData;
// 	IKinemaReplicateData	FrameSendData;
};

