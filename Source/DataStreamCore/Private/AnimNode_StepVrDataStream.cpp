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

	RedundanceMocapBones.Add(FBoneReference(TEXT("Spine")));
	RedundanceMocapBones.Add(FBoneReference(TEXT("Spine2")));
	RedundanceMocapBones.Add(FBoneReference(TEXT("neck1")));
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
		mSkeletonBinding.BindToSkeleton(AnimInstanceProxy,BindMocapBones);
		mSkeletonBinding.BindToHandSkeleton(AnimInstanceProxy, BindMocapHandBones);
		mSkeletonBinding.BindToFaceMorghTarget(AnimInstanceProxy, FaceMorphTargetName);

		for (auto& Ref : RedundanceMocapBones)
		{
			Ref.Initialize(skeleton);
		}

		//for (auto& Ref : FaceMorphTargetName)
		//{
		//	if (!BindMorphTarget.Contains(Ref))
		//	{
		//		BindMorphTarget.Add(Ref, FStepFaceMorghs::Expressions_abdomExpansion_max);
		//	}
		//}
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
	EvaluateGraphExposedInputs.Execute(Context);

	//更新数据
	do 
	{
		if (!mSkeletonBinding.IsConnected())
		{
			break;
		}
		if (mSkeletonBinding.IsBodyBound())
		{
			mSkeletonBinding.UpdateBodyFrameData(BonesData);
		}
		if (mSkeletonBinding.IsHandBound())
		{
			mSkeletonBinding.UpdateHandFrameData(HandBonesData);
		}
		if (mSkeletonBinding.IsFaceBound())
		{
			mSkeletonBinding.UpdateFaceFrameData(FaceMorphTargetData);
		}
	} while (0);
}

//FGraphEventRef oExecOnGameThread(TFunction<void()> funcLambda)
//{
//	FGraphEventRef funcTask = FFunctionGraphTask::CreateAndDispatchWhenReady(funcLambda, TStatId(), NULL, ENamedThreads::GameThread);
//	return funcTask;
//}

void FAnimNode_StepDataStream::Evaluate_AnyThread(FPoseContext& Output)
{
	check(Output.AnimInstanceProxy->GetSkeleton() != nullptr);
	Output.Pose.ResetToRefPose();

	//更新动捕姿态
	do 
	{
		if (!mSkeletonBinding.IsBodyBound())
		{
			break;
		}

		for (int32 i = 0; i < BonesData.Num(); i++)
		{
			auto ue4Index = mSkeletonBinding.GetUE4BoneIndex(i);

			if (ue4Index != INDEX_NONE)
			{
				FTransform& bone = BonesData[i];

				auto SkeletonIndex = Output.Pose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(ue4Index));

				Output.Pose[SkeletonIndex] = bone;
			}
		}

		//多余的骨骼清零
		for (auto& BoneRef : RedundanceMocapBones)
		{
			if (!BoneRef.HasValidSetup())
			{
				continue;
			}

			auto SkeletonIndex = Output.Pose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneRef.BoneIndex));
			Output.Pose[SkeletonIndex] = FTransform::Identity;
		}
	} while (0);


	//更新手部骨骼姿态
	do{
		
		if (!mSkeletonBinding.IsHandBound())
		{
			break;
		}

		for (int32 i = 0; i < HandBonesData.Num(); i++)
		{
			auto ue4Index = mSkeletonBinding.GetUE4HandBoneIndex(i);

			if (ue4Index != INDEX_NONE)
			{
				auto SkeletonIndex = Output.Pose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(ue4Index));

				if (1/*i == 0*/)
				{
					Output.Pose[SkeletonIndex].SetRotation(HandBonesData[i].Quaternion());
				}
				//Output.Pose[SkeletonIndex].SetRotation(HandBonesData[i].Quaternion());
				//Output.Pose[SkeletonIndex].SetRotation(FQuat::Identity); 
			}
		}
	} while (0);


	//更新面部捕捉
	do 
	{
		if (!mSkeletonBinding.IsFaceBound())
		{
			break;
		}

		USkeletalMeshComponent* SkeletonComponet = Output.AnimInstanceProxy->GetSkelMeshComponent();
		if (SkeletonComponet == nullptr)
		{
			break;
		}

		static UEnum* GRootEnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("FStepFaceMorghs"), true);
		for (auto& temp : FaceMorphTargetName)
		{
			if (GRootEnumPtr == nullptr)
			{
				break;
			}

			FStepFaceMorghs* EnumPtr = BindMorphTarget.Find(temp);
			if (EnumPtr == nullptr)
			{
				continue;
			}

			FString CurShooterDataStr(GRootEnumPtr->GetNameByValue((int)(*EnumPtr)).ToString());
			float* ValuePtr = FaceMorphTargetData.Find(CurShooterDataStr);
			if (ValuePtr == nullptr)
			{
				continue;
			}

			SkeletonComponet->SetMorphTarget(FName(*CurShooterDataStr), *ValuePtr);
		}

	} while (0);
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
