#pragma once

#include "StepVrStream.h"
#include "StepMocapDefine.h"
#include "AnimNode_StepVrDataStream.generated.h"


class UStepReplicatedComponent;


USTRUCT(BlueprintInternalUseOnly)
struct  STEPVRDATASTREAMCORE_API FAnimNode_StepDataStream : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

	//FPoseLink InPose;
	
	/**
	 * 是否关闭全身捕捉
	 * Override : EnableHand
	 */
	UPROPERTY(EditAnywhere, Category = StepServer)
	bool StopSkeletonCapture = false;
	
	/**
	* 是否开启手部捕捉
	*/
	UPROPERTY(EditAnywhere, Category = StepServer)
	bool EnableHand = false;

	/**
	* 是否开启面部捕捉
	*/
	UPROPERTY(EditAnywhere, Category = StepServer)
	bool EnableFace = false;

	/**
	* 骨骼没有Skt文件，填空，使用BindMocapBones|BindMocapHandBones
	* 骨骼有Skt文件，设置对应文件名，文件存放path:\Plugins\StepVrMocap\ThirdParty\skt\
	*/
	UPROPERTY(EditAnywhere, Category = StepServer)
	FString SktName = TEXT("");

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

	/**
	 * 服务器IP
	 * 支持局域网IP
	 */
	UPROPERTY(EditAnywhere, Category=Server/*, meta=(PinShownByDefault)*/)
	FName ServerName = TEXT("127.0.0.1");

	/**
	 * 是否根据TPOSE缩放骨骼
	 */
	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault))
	bool ApplyScale = false;

	/**
	 * 服务器端口号
	 * 暂时默认无需修改
	 */
	UPROPERTY(EditAnywhere, Category=Server)
	int32 PortNumber = 9516;

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
	FMocapServerInfo BuildServerInfo();
	void CheckInit();

	//绑定骨架
	FStepDataToSkeletonBinding mSkeletonBinding;

	//缓存数据
	const FAnimInstanceProxy*	CacheAnimInstanceProxy = nullptr;
	const AActor*				CacheOwnerActor = nullptr;
	FVector						CacheSkeletonScale;

	//联机
	FStepControllState StepControllState = FStepControllState::Local_Replicate_N;
	bool IsInit = false;
	uint32 AddrValue = 0;
};

