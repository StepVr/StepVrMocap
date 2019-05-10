// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#include "AnimNode_StepVrDataStream.h"
#include "Animation/AnimInstanceProxy.h"
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

		for (auto& Ref : RedundanceMocapBones)
		{
			Ref.Initialize(skeleton);
		}
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

	MocapServerInfo.ServerIP = CheckConert2LocalIP(ServerName.ToString());
	MocapServerInfo.ServerPort = PortNumber;

	BindServerStream(Context.AnimInstanceProxy);
}


void FAnimNode_StepDataStream::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FAnimNode_StepDataStream::Update_AnyThread"), STAT_StepMocapUpdate, STATGROUP_Game);

	InPose.Update(Context);
	EvaluateGraphExposedInputs.Execute(Context);

	DataIsReady = false;
	//更新数据
	do 
	{
		if (!mSkeletonBinding.IsConnected())
		{
			break;
		}
		if (mSkeletonBinding.IsBound())
		{
			mSkeletonBinding.UpdateBodyFrameData(BonesData);
		}
		if (mSkeletonBinding.IsHandBound())
		{
			mSkeletonBinding.UpdateHandFrameData(HandBonesData);
		}

		DataIsReady = true;
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

	if (!DataIsReady)
	{
		return;
	}

	//更新动捕姿态
	do 
	{
		if (BonesData.Num() != STEPBONESNUMS)
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
		
		if (HandBonesData.Num() != STEPHANDBONESNUMS)
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
}

void FAnimNode_StepDataStream::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	InPose.CacheBones(Context);
}
void FAnimNode_StepDataStream::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	Super::OnInitializeAnimInstance(InProxy, InAnimInstance);

	OwnerPawn = Cast<APawn>(InAnimInstance->GetOwningActor());	
	CacheAnimInstanceProxy = InProxy;
}

void FAnimNode_StepDataStream::InitData()
{
}

FString FAnimNode_StepDataStream::CheckConert2LocalIP(const FString& IP)
{
	FString Addr = IP;
	if (IP.Equals(TEXT("127.0.0.1"),ESearchCase::IgnoreCase) || 
		IP.Equals(TEXT("localhost"), ESearchCase::IgnoreCase))
	{
		bool CanBind = false;
		TSharedRef<FInternetAddr> LocalIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, CanBind);
		if (LocalIp->IsValid())
		{
			Addr = LocalIp->ToString(false);
		}
	}

	return Addr;
}

