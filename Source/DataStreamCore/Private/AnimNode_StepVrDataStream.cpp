// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#include "AnimNode_StepVrDataStream.h"
#include "StepVrReplicatedComponent.h"

#include "Animation/AnimInstanceProxy.h"
#include "Animation/MorphTarget.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"




FAnimNode_StepDataStream::FAnimNode_StepDataStream()
{
	for (FString Name : StepBoneNames)
	{
		BindMocapBones.FindOrAdd(Name) = FBoneReference(*Name);
	}
	for (FString Name : StepHandBoneNames)
	{
		BindMocapHandBones.FindOrAdd(Name) = FBoneReference(*Name);
	}

	for (auto& Temp : StepFaceMorphTargets)
	{
		BindMorphTarget.Add(Temp, Temp);
	}
}

FAnimNode_StepDataStream::~FAnimNode_StepDataStream()
{

}


void FAnimNode_StepDataStream::BindSkeleton(FAnimInstanceProxy* AnimInstanceProxy)
{
	if (AnimInstanceProxy == nullptr)
	{
		return;
	}

	USkeleton* skeleton = AnimInstanceProxy->GetSkeleton();
	if (skeleton == nullptr)
	{
		return;
	}

	if (mSkeletonBinding.ConnectToServer(MocapServerInfo))
	{
		mSkeletonBinding.BindToSkeleton(AnimInstanceProxy,BindMocapBones, BindMocapHandBones);
	}
}

void FAnimNode_StepDataStream::BindServerStream(FAnimInstanceProxy* AnimInstanceProxy)
{
	if (MocapServerInfo.IsEmpty())
	{
		return;
	}

	if (AnimInstanceProxy == nullptr)
	{
		return;
	}

	BindSkeleton(AnimInstanceProxy);
}

void FAnimNode_StepDataStream::IntializeServerStreamer(FAnimInstanceProxy* AnimInstanceProxy)
{
	if (MocapServerInfo.IsEmpty())
	{
		return;
	}

	if (AnimInstanceProxy == nullptr)
	{
		return;
	}

	BindSkeleton(AnimInstanceProxy);
}

void FAnimNode_StepDataStream::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);
	InPose.Initialize(Context);

	// Forward to the incoming pose link.
	check(Context.AnimInstanceProxy != nullptr);

	MocapServerInfo.ServerIP = Convert2LocalIP(ServerName.ToString());
	MocapServerInfo.ServerPort = PortNumber;
	//MocapServerInfo.EnableBody = EnableBody;
	MocapServerInfo.EnableHand = EnableHand;
	MocapServerInfo.EnableFace = EnableFace;


	BindServerStream(Context.AnimInstanceProxy);
}


void FAnimNode_StepDataStream::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FAnimNode_StepDataStream::Update_AnyThread"), STAT_StepMocapUpdate, STATGROUP_Game);

	if (!bReplicatedComponent)
	{
		InitReplicateComponet();
	}

	InPose.Update(Context);

#if (ENGINE_MAJOR_VERSION>=4) && (ENGINE_MINOR_VERSION>=22)
	GetEvaluateGraphExposedInputs().Execute(Context);
#else
	EvaluateGraphExposedInputs.Execute(Context);
#endif
	

	if (mSkeletonBinding.IsConnected())
	{
		//骨骼数据
		mSkeletonBinding.UpdateSkeletonFrameData();

		//面部数据
		if (EnableFace)
		{
			mSkeletonBinding.UpdateFaceFrameData(FaceMorphTargetData);
		}
	}
}

//FGraphEventRef oExecOnGameThread(TFunction<void()> funcLambda)
//{
//	FGraphEventRef funcTask = FFunctionGraphTask::CreateAndDispatchWhenReady(funcLambda, TStatId(), NULL, ENamedThreads::GameThread);
//	return funcTask;
//}

void FAnimNode_StepDataStream::EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output)
{
	Output.ResetToRefPose();

	//更新动捕姿态
	const FBoneContainer& RequiredBone = Output.AnimInstanceProxy->GetRequiredBones();
	int32 NumBones = RequiredBone.GetNumBones();
	for (int32 Index = 0, StepIndex = 0; Index < NumBones; Index++)
	{
		auto MapBoneData = mSkeletonBinding.GetUE4BoneIndex(StepIndex);

		//根节点
		if (Index == 0)
		{
			FCompactPoseBoneIndex BoneIndex(Index);
			Output.Pose.SetComponentSpaceTransform(BoneIndex, FTransform::Identity);
			continue;
		}

		//没有匹配的点
		if (MapBoneData.UeBoneIndex != Index)
		{
			FCompactPoseBoneIndex BoneIndex(Index);
			Output.Pose.CalculateComponentSpaceTransform(BoneIndex);
			continue;
		}

		//匹配的点
		FCompactPoseBoneIndex BoneIndex(MapBoneData.UeBoneIndex);

		switch (MapBoneData.MapBoneType)
		{
		case FStepDataToSkeletonBinding::EMapBoneType::Bone_Body:
		{
			MapBoneData.BoneData.SetToRelativeTransform(FTransform::Identity);
		}
		break;
		case FStepDataToSkeletonBinding::EMapBoneType::Bone_Hand:
		{
			MapBoneData.BoneData.SetToRelativeTransform(FTransform::Identity);
			auto Temp = Output.Pose.GetComponentSpaceTransform(BoneIndex);
			MapBoneData.BoneData.SetLocation(Temp.GetLocation());
		}
		break;
		}

		Output.Pose.SetComponentSpaceTransform(BoneIndex, MapBoneData.BoneData);
		StepIndex++;
	}

}

void FAnimNode_StepDataStream::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	InPose.CacheBones(Context);
}

void FAnimNode_StepDataStream::OverrideAsset(class UAnimationAsset* NewAsset)
{
// 	USkeletalMesh* Skeleton= NewAsset->GetPreviewMesh();
// 
// 	for (int32 i = 0; i < Skeleton->MorphTargets.Num(); i++)
// 	{
// 		FString MorghName = Skeleton->MorphTargets[i]->GetName();
// 		BindMorphTarget.Add(MorghName,FStepFaceMorghs::Expressions_abdomExpansion_max);
// 	}
}

void FAnimNode_StepDataStream::PostCompile(const USkeleton * InSkeleton)
{
	//BindMorphTarget.Add(TEXT("asdasd"), FStepFaceMorghs::Expressions_abdomExpansion_max);
}

void FAnimNode_StepDataStream::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	Super::OnInitializeAnimInstance(InProxy, InAnimInstance);

	OwnerPawn = Cast<APawn>(InAnimInstance->GetOwningActor());	

	CacheAnimInstanceProxy = InProxy;
}

void FAnimNode_StepDataStream::InitReplicateComponet()
{
	if (OwnerPawn == nullptr)
	{
		return;
	}

	if (!OwnerPawn->HasActorBegunPlay())
	{
		return;
	}

	ReplicatedComponent = Cast<UStepReplicatedComponent>(OwnerPawn->GetComponentByClass(UStepReplicatedComponent::StaticClass()));
	if (ReplicatedComponent == nullptr)
	{
		bReplicatedComponent = true;
		return;
	}

	if (ReplicatedComponent->PlayerAddr.Equals(REPLICATE_NONE))
	{
		return;
	}

	MocapServerInfo.ServerIP = Convert2LocalIP(ReplicatedComponent->PlayerAddr);
	mSkeletonBinding.ConnectToServer(MocapServerInfo);
	bReplicatedComponent = true;
}
