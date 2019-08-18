// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.


#pragma once

#include "StepVrStream.h"
#include "StepMocapDefine.h"
#include "AnimNode_StepVrDataStream.generated.h"


class UStepReplicatedComponent;


USTRUCT(BlueprintType)
struct  STEPVRDATASTREAMCORE_API FAnimNode_StepDataStream : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

	//FPoseLink InPose;
	
	/**
	 * 暂停骨骼动捕
	 */
	UPROPERTY(EditAnywhere, Category = StepMocapBindBones)
	bool	PauseSkeletonCapture = false;

	/**
	 * Step修改身体 22 根骨骼点
	 * PS1:骨骼结构和Step标准骨骼结构相同，只需要匹配名字所对应得骨骼即可
	 * PS2:骨骼结构和Step标准骨骼结构不相同，需要Step对骨骼进行映射
	 */
	UPROPERTY(EditAnywhere, Category = StepMocapBindBones)
	TMap<FString, FBoneReference>	BindMocapBones;

	/**
	* Step修改手部 32 根骨骼点
	* PS1:手部只需要关节匹配
	*/
	UPROPERTY(EditAnywhere, Category = StepMocapBindBones)
	TMap<FString, FBoneReference>	BindMocapHandBones;

	/**
	 * Step只支持配置文件对应得MorphTarget（C://StepFace//config//configBlends.json）
	 * 骨架多余得MorphTarget将会被忽略
	 * Map Key	 : Step MorphTarget
	 * Map Value : Skeleton MorphTarget
	 */
	UPROPERTY(EditAnywhere, Category = StepMocapBindBones)
	TMap<FString, FString>	BindMorphTarget;

	UPROPERTY(EditAnywhere, Category=Server, meta=(PinShownByDefault))
	FName ServerName = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, Category=Server,meta=(PinShownByDefault))
	int32 PortNumber = 9516;

	//UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault, ToolTip = "Apply character scale from Step"))
	//bool ApplyStepScale = false;

	/**
	 * 是否开启身体捕捉
	 */
	//UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault, ToolTip = "Apply character scale from Step"))
	//bool EnableBody = true;

	/**
	* 是否开启手部捕捉
	*/
	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault, ToolTip = "Apply character scale from Step"))
	bool EnableHand = false;

	/**
	* 是否开启面部捕捉
	*/
	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault, ToolTip = "Apply character scale from Step"))
	bool EnableFace = false;

public:	

	FAnimNode_StepDataStream();
	~FAnimNode_StepDataStream();

	void Connected();
	void BindSkeleton(FAnimInstanceProxy* AnimInstanceProxy);
	
	// FAnimNode_Base interface
	void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output) override;
	//void CacheBones_AnyThread(const FAnimationCacheBonesContext & Context) override;
	// End of FAnimNode_Base interface
	
	virtual void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;

private:
	void BuildServerInfo();
	void CheckInit();

	//绑定骨架
	FStepDataToSkeletonBinding mSkeletonBinding;

	//链接server属性
	FMocapServerInfo MocapServerInfo;

	//缓存数据
	const FAnimInstanceProxy* CacheAnimInstanceProxy = nullptr;
	const APawn* OwnerPawn = nullptr;

	//联机
	bool IsInit = false;
	bool IsLocal = true;
	uint32 AddrValue = 0;
};

