// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#include "AnimNode_StepVrDataStream.h"
#include "StepVrReplicatedComponent.h"
#include "StepVrComponent.h"
#include "StepVrSkt.h"


#include "Animation/AnimInstanceProxy.h"
#include "Animation/MorphTarget.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "CoreMiscDefines.h"




FAnimNode_StepDataStream::FAnimNode_StepDataStream()
{
	//身体骨骼
	for (auto& Name : StepBoneNames)
	{
		BindMocapBones.FindOrAdd(Name) = FBoneReference(*Name);
	}

	//手部骨骼
	for (auto& Name : StepHandBoneNames)
	{
		BindMocapHandBones.FindOrAdd(Name) = FBoneReference(*Name);
	}

	//for (int32 Index = 0; Index < StepHandBoneNames.Num(); Index++)
	//{
	//	if (StepHandBoneDelete.Find(Index) == INDEX_NONE)
	//	{
	//		BindMocapHandBones.FindOrAdd(StepHandBoneNames[Index]) = FBoneReference(*StepHandBoneNames[Index]);
	//	}
	//}

	//面部顶点
	for (auto& Temp : StepFaceMorphTargets)
	{
		BindMorphTarget.Add(Temp, Temp);
	}
}

FAnimNode_StepDataStream::~FAnimNode_StepDataStream()
{

}


void FAnimNode_StepDataStream::Connected()
{
	BuildServerInfo();
	mSkeletonBinding.ConnectToServer(MocapServerInfo);
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

	//绑定骨骼
	auto Skeletons = STEPVRSKT->GetSktRetarget(SktName);
	if(Skeletons.Num() == (STEPHANDBONESNUMS + STEPBONESNUMS)){

		int32 Index = 0;
		
		//身体骨骼
		for (auto& Name : StepBoneNames)
		{
			BindMocapBones.FindOrAdd(Name) = FBoneReference(*Skeletons[Index]);
			Index++;
		}

		//手部骨骼
		for (auto& Name : StepHandBoneNames)
		{
			BindMocapHandBones.FindOrAdd(Name) = FBoneReference(*Skeletons[Index]);
			Index++;
		}
	}

	if (!EnableHand)
	{
		BindMocapHandBones.Empty();
	}
	mSkeletonBinding.BindToSkeleton(AnimInstanceProxy, BindMocapBones, BindMocapHandBones);

	//绑定顶点变形
	mSkeletonBinding.BindToFaceMorghTarget(AnimInstanceProxy, BindMorphTarget);
}

void FAnimNode_StepDataStream::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	INC_DWORD_STAT(COUNT_Initialize_AnyThread);
	SCOPE_CYCLE_COUNTER(STAT_Initialize_AnyThread);

	FAnimNode_Base::Initialize_AnyThread(Context);

	// Forward to the incoming pose link.
	check(Context.AnimInstanceProxy != nullptr);

	//绑定骨骼
	BindSkeleton(Context.AnimInstanceProxy);

#if WITH_EDITOR
	if (GWorld->WorldType == EWorldType::Editor)
	{
		IsInit = true;
		StepControllState = FStepControllState::Local_Replicate_N;
		Connected();
	}
#endif
}


void FAnimNode_StepDataStream::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	SCOPE_CYCLE_COUNTER(STAT_Update_AnyThread);

#if (ENGINE_MAJOR_VERSION>=4) && (ENGINE_MINOR_VERSION>=22)
	GetEvaluateGraphExposedInputs().Execute(Context);
#else
	EvaluateGraphExposedInputs.Execute(Context);
#endif

	if (!IsInit)
	{
		CheckInit();
		return;
	}

	//骨骼数据
	mSkeletonBinding.UpdateSkeletonFrameData();

	//面部数据
	mSkeletonBinding.UpdateFaceFrameData();
}

//FGraphEventRef oExecOnGameThread(TFunction<void()> funcLambda)
//{
//	FGraphEventRef funcTask = FFunctionGraphTask::CreateAndDispatchWhenReady(funcLambda, TStatId(), NULL, ENamedThreads::GameThread);
//	return funcTask;
//}

void FAnimNode_StepDataStream::EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output)
{
	SCOPE_CYCLE_COUNTER(STAT_EvaluateComponentSpace_AnyThread);

	Output.ResetToRefPose();

	if (!IsInit)
	{
		return;
	}

	//更新动捕姿态
	if (StopSkeletonCapture == false)
	{
		auto AllUpdateBones = mSkeletonBinding.GetUE4NeedUpdateBones();
		//const FBoneContainer& RequiredBone = Output.AnimInstanceProxy->GetRequiredBones();
		//int32 NumBones = RequiredBone.GetNumBones();
		for (int32 Index = 0, StepIndex = 0; Index < AllUpdateBones.Num(); Index++)
		{
			auto MapBoneData = mSkeletonBinding.GetUE4BoneIndex(StepIndex);
			int32 UEIndex = AllUpdateBones[Index];

			//根节点
			if (UEIndex == 0)
			{
				FCompactPoseBoneIndex BoneIndex(0);
				Output.Pose.SetComponentSpaceTransform(BoneIndex, FTransform::Identity);
				continue;
			}

			//没有匹配的点
			if (MapBoneData.UeBoneIndex != UEIndex)
			{
				FCompactPoseBoneIndex BoneIndex(UEIndex);
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


	//更新面部捕捉
	USkeletalMeshComponent* SkeletonComponet = Output.AnimInstanceProxy->GetSkelMeshComponent();
	if (SkeletonComponet)
	{
		auto FaceBindData = mSkeletonBinding.GetUE4FaceData();
		for (auto& TempItr : FaceBindData)
		{
			SkeletonComponet->SetMorphTarget(TempItr.UE4MarphName, TempItr.MorphValue);
		}
	}
}

//void FAnimNode_StepDataStream::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
//{
//	FAnimNode_Base::CacheBones_AnyThread(Context);
//	InPose.CacheBones(Context);
//}

void FAnimNode_StepDataStream::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	Super::OnInitializeAnimInstance(InProxy, InAnimInstance);

	OwnerPawn = Cast<APawn>(InAnimInstance->GetOwningActor());	

	CacheAnimInstanceProxy = InProxy;
}

void FAnimNode_StepDataStream::BuildServerInfo()
{
	//MocapServerInfo.ServerIP = Convert2LocalIP(ServerName.ToString());
	MocapServerInfo.ServerIP = ServerName.ToString();
	MocapServerInfo.ServerPort = PortNumber;
	//MocapServerInfo.EnableBody = EnableBody;

	MocapServerInfo.EnableHand = EnableHand;
	MocapServerInfo.EnableFace = EnableFace;

	MocapServerInfo.StepControllState = StepControllState;
	MocapServerInfo.AddrValue = AddrValue;
}

void FAnimNode_StepDataStream::CheckInit()
{
	IsInit = false;

	do 
	{
		if (!IsValid(OwnerPawn))
		{
			//Finish 不是角色
			IsInit = true;
			StepControllState = FStepControllState::Local_Replicate_N;
			break;
		}

		if (!OwnerPawn->HasActorBegunPlay())
		{
			break;
		}

		TArray<UStepVrComponent*> Coms;
		OwnerPawn->GetComponents(Coms);
		if (Coms.Num() == 0)
		{
			//Finish 没有添加组件，无法同步
			IsInit = true;
			StepControllState = FStepControllState::Local_Replicate_N;
			break;
		}

		UStepVrComponent* TargetCom = Coms[0];
		if (!TargetCom->bMocapReplicate)
		{
			//Finish 不需要同步
			IsInit = true;
			StepControllState = FStepControllState::Local_Replicate_N;
			break;
		}

		if (!TargetCom->IsInitialization())
		{
			break;
		}

		if (TargetCom->IsLocalControlled())
		{
			//Finish 本地角色
			IsInit = true;
			StepControllState = FStepControllState::Local_Replicate_Y;
			break;
		}

		if (!TargetCom->IsValidPlayerAddr())
		{
			break;
		}

		IsInit = true;
		AddrValue = TargetCom->GetPlayerAddr();
		StepControllState = FStepControllState::Remote_Replicate_Y;	
	} while (0);

	if (IsInit)
	{
		Connected();
	}
}
