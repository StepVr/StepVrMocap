#pragma once

#include "StepVrStream.h"
#include "StepMocapDefine.h"

#include "LiveLinkRetargetAsset.h"
#include "AnimNode_StepVrDataStream.generated.h"


class UStepReplicatedComponent;


USTRUCT(BlueprintType)
struct  STEPVRDATASTREAMCORE_API FAnimNode_StepDataStream : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()


public:
	FAnimNode_StepDataStream();
	~FAnimNode_StepDataStream();

	//全局动画节点
	static FAnimNode_StepDataStream* GetStepDataStream(uint32 ActorGUID);
	static void RegistStepDataStream(uint32 ActorGUID, FAnimNode_StepDataStream* Target);
	static void UnRegistStepDataStream(uint32 ActorGUID);
	static TMap<uint32, FAnimNode_StepDataStream*> RegistStepDataStreams;
	void MocapUpdateSkt();
	void MocapTPose();
	void MocapSetNewIP(const FString& InData);
	void MocapSetNewFaceID(const FString& InData);
	void MocapSetNewFaceScale(float InData);
	FVector MocapGetSkeletonScale();


public:
	//FPoseLink InPose;
	
	/**
	 * 服务器IP
	 * 支持局域网IP
	 */
	UPROPERTY(EditAnywhere, Category = StepServer, meta = (PinShownByDefault))
	FName ServerName = TEXT("127.0.0.1");

	/**
	 * 服务器端口号
	 * 暂时默认无需修改
	 */
	UPROPERTY(EditAnywhere, Category = StepServer)
	int32 ServerPort = 9516;

	/**
	* 是否适配人物实际大小
	*/
	UPROPERTY(EditAnywhere, Category = StepServer, meta = (PinShownByDefault))
	bool ApplyScale = false;

	/**
	 * 是否暂停全身捕捉
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
	* 骨骼没有Skt文件，填空，使用BindMocapBones|BindMocapHandBones
	* 骨骼有Skt文件，设置对应文件名，文件存放path:\Plugins\StepVrMocap\ThirdParty\skt\
	*/
	UPROPERTY(EditAnywhere, Category = StepServer)
	FString SktName = TEXT("");

	/**
	 * 自动切换skt
	 * 一台电脑只能存在一个skt
	 */
	UPROPERTY(EditAnywhere, Category = StepServer)
	bool AutoChangeSkt = true;

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
	 * ARFace面部捕捉
	 */
	//是否开启面部捕捉
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = ARFace)
	bool EnableFace = false;

	//重定向曲线
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, Category = ARFace, meta = (NeverAsPin))
	TSubclassOf<ULiveLinkRetargetAsset> RetargetAsset;

	//ARFace ID, PS:Iphone标题
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, Category = ARFace, meta = (NeverAsPin))
	FName FaceID;

	UPROPERTY(transient)
	ULiveLinkRetargetAsset* CurrentRetargetAsset = nullptr;

public:	
	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual bool HasPreUpdate() const { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	//void CacheBones_AnyThread(const FAnimationCacheBonesContext & Context) override;
	// End of FAnimNode_Base interface
	
	virtual void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;

private:
	void Connected();
	void BindSkeleton(FAnimInstanceProxy* AnimInstanceProxy);
	

	//绑定骨架
	FStepDataToSkeletonBinding mSkeletonBinding;

	//缓存数据
	const FAnimInstanceProxy*	CacheAnimInstanceProxy = nullptr;
	int32						CacheGUID = 0;

	//缩放
	float						CachedSkeletonScaleDeltaTime = 0.f;
	FVector						CacheSkeletonScale = FVector::OneVector;
	//联机
	FStepControllState StepControllState = FStepControllState::Local_UnReplicate;

	//是否连接
	bool bConnected = false;

	//面部更新间隔
	float CachedDeltaTime = 0.f;
};

