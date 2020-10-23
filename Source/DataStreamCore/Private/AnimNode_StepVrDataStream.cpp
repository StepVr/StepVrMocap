#include "AnimNode_StepVrDataStream.h"
#include "StepVrDataStreamModule.h"
#include "StepVrFaceSource.h"
#include "StepMocapComponent.h"
#include "StepVrSkt.h"


#include "Animation/AnimInstanceProxy.h"
#include "Animation/MorphTarget.h"
#include "AnimationRuntime.h"

#include "SocketSubsystem.h"
#include "IPAddress.h"

#include "Misc/CoreMiscDefines.h"
#include "Async/Async.h"

#include "Launch/Resources/Version.h"


TMap<uint32, FAnimNode_StepDataStream*> FAnimNode_StepDataStream::RegistStepDataStreams;
FAnimNode_StepDataStream* FAnimNode_StepDataStream::GetStepDataStream(uint32 ActorGUID)
{
	auto StepDataStream = RegistStepDataStreams.Find(ActorGUID);
	if (StepDataStream)
	{
		return *StepDataStream;
	}
	return nullptr;
}

void FAnimNode_StepDataStream::RegistStepDataStream(uint32 ActorGUID, FAnimNode_StepDataStream* Target)
{
	RegistStepDataStreams.FindOrAdd(ActorGUID) = Target;
}

void FAnimNode_StepDataStream::UnRegistStepDataStream(uint32 ActorGUID)
{
	if (ActorGUID != 0)
	{
		RegistStepDataStreams.Remove(ActorGUID);
	}
} 

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
	FAnimNode_StepDataStream::UnRegistStepDataStream(CacheGUID);
}


void FAnimNode_StepDataStream::MocapUpdateSkt()
{
	auto TempStream = mSkeletonBinding.GetMocapStream();

	if (TempStream.IsValid() && (!SktName.IsEmpty()))
	{
		TempStream->ReplcaeSkt(SktName);
	}
}

void FAnimNode_StepDataStream::MocapTPose()
{
	auto TempStream = mSkeletonBinding.GetMocapStream();

	if (TempStream.IsValid())
	{
		TempStream->TPose();
	}
}

void FAnimNode_StepDataStream::MocapSetNewIP(const FString& InData)
{
	ServerName = FName(*InData);
	Connected();
}

void FAnimNode_StepDataStream::MocapSetNewFaceID(const FString& InData)
{
	FaceID = FName(*InData);
}

void FAnimNode_StepDataStream::MocapSetNewFaceScale(float InData)
{
	if (FStepListenerToAppleARKit* StepListener = FStepDataStreamModule::GetStepListenerToAppleARKit())
	{
		StepListener->SetFaceScale(FaceID,InData);
	}
}

FVector FAnimNode_StepDataStream::MocapGetSkeletonScale()
{
	return CacheSkeletonScale;
}

void FAnimNode_StepDataStream::Connected()
{
	FMocapServerInfo MocapServerInfo;

	MocapServerInfo.ServerIP = ServerName.ToString();
	MocapServerInfo.ServerPort = ServerPort;

	MocapServerInfo.EnableHand = EnableHand;
	MocapServerInfo.EnableFace = EnableFace;

	MocapServerInfo.StepControllState = StepControllState;
	MocapServerInfo.SktName = SktName;

	bConnected = mSkeletonBinding.ConnectToServer(MocapServerInfo);
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
	if (!SktName.IsEmpty())
	{
		auto Skeletons = STEPVRSKT->GetSktRetarget(SktName);
		if (Skeletons.Num() == (STEPHANDBONESNUMS + STEPBONESNUMS)) {

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
	}

	if (!EnableHand)
	{
		BindMocapHandBones.Empty();
	}
	mSkeletonBinding.BindToSkeleton(AnimInstanceProxy, BindMocapBones, BindMocapHandBones);
	
	//if (EnableFace)
	//{
	//	//绑定顶点变形 
	//	mSkeletonBinding.BindToFaceMorghTarget(AnimInstanceProxy, BindMorphTarget);
	//}
}

void FAnimNode_StepDataStream::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	INC_DWORD_STAT(COUNT_Initialize_AnyThread);
	SCOPE_CYCLE_COUNTER(STAT_Initialize_AnyThread);

	FAnimNode_Base::Initialize_AnyThread(Context);

	// Forward to the incoming pose link.
	check(Context.AnimInstanceProxy != nullptr);

	//动画蓝图代理
	CacheAnimInstanceProxy = Context.AnimInstanceProxy;

	//绑定骨骼
	BindSkeleton(Context.AnimInstanceProxy);

	StepControllState = FStepControllState::Local_UnReplicate;
	Connected();


	//归属Actor
	if (auto SkeletonMesh = CacheAnimInstanceProxy->GetSkelMeshComponent())
	{
		CacheGUID = SkeletonMesh->GetOwner()->GetUniqueID();
		if (CacheGUID > 0)
		{
			FAnimNode_StepDataStream::RegistStepDataStream(CacheGUID, this);
		}
		if (AutoChangeSkt)
		{
			MocapUpdateSkt();
		}
	}
}


void FAnimNode_StepDataStream::PreUpdate(const UAnimInstance* InAnimInstance)
{
	Super::PreUpdate(InAnimInstance);

	//面部Retarget
	if (CurrentRetargetAsset == nullptr)
	{
		if (UClass* RetargetAssetPtr = RetargetAsset.Get())
		{
			CurrentRetargetAsset = NewObject<ULiveLinkRetargetAsset>(const_cast<UAnimInstance*>(InAnimInstance), RetargetAssetPtr);
			CurrentRetargetAsset->Initialize();
		}
	}
}

void FAnimNode_StepDataStream::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	SCOPE_CYCLE_COUNTER(STAT_Update_AnyThread);

#if (ENGINE_MAJOR_VERSION>=4) && (ENGINE_MINOR_VERSION>=22)
	GetEvaluateGraphExposedInputs().Execute(Context);
#else
	EvaluateGraphExposedInputs.Execute(Context);
#endif

	//骨骼数据
	mSkeletonBinding.UpdateSkeletonFrameData();

	//面部数据
	mSkeletonBinding.UpdateFaceFrameData();

	CachedDeltaTime += Context.GetDeltaTime();
	CachedSkeletonScaleDeltaTime += Context.GetDeltaTime();
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

	if (bConnected == false)
	{
		return;
	}

	//Scale 
	CacheSkeletonScale = mSkeletonBinding.GetSkeletonScale();

	//更新动捕姿态
	if (StopSkeletonCapture == false)
	{
		//修改骨骼
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
				if (CacheSkeletonScale.X > 0.1)
				{
					MapBoneData.BoneData.ScaleTranslation(1.f / CacheSkeletonScale.X);
				}
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

	//Scale
	if (GWorld && GWorld->WorldType != EWorldType::Editor)
	{
		if (ApplyScale && CachedSkeletonScaleDeltaTime > 1)
		{
			CachedSkeletonScaleDeltaTime = 0.f;
			AsyncTask(ENamedThreads::GameThread, [&]()
				{
					if (CacheAnimInstanceProxy && CacheAnimInstanceProxy->GetSkelMeshComponent())
					{
						CacheAnimInstanceProxy->GetSkelMeshComponent()->SetWorldScale3D(CacheSkeletonScale);
					}
				});
		}
	}


	//更新面部捕捉
	if (EnableFace)
	{
		if (FStepListenerToAppleARKit* StepListener = FStepDataStreamModule::GetStepListenerToAppleARKit())
		{
			FLiveLinkBaseStaticData* BaseStaticData = StepListener->GetStaticData(FaceID);
			FLiveLinkBaseFrameData* BaseFrameData = StepListener->GetFrameData(FaceID);

			if (CurrentRetargetAsset && BaseStaticData && BaseFrameData)
			{
				auto CompactPose = Output.Pose.GetPose();
				CurrentRetargetAsset->BuildPoseAndCurveFromBaseData(CachedDeltaTime, BaseStaticData, BaseFrameData, CompactPose, Output.Curve);
				// Reset so that if we evaluate again we don't "create" time inside of the retargeter
				CachedDeltaTime = 0.f;
			}
		}
	}
}

void FAnimNode_StepDataStream::GatherDebugData(FNodeDebugData& DebugData)
{
	Super::GatherDebugData(DebugData);
}

//void FAnimNode_StepDataStream::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
//{
//	FAnimNode_Base::CacheBones_AnyThread(Context);
//	InPose.CacheBones(Context);
//}

void FAnimNode_StepDataStream::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	Super::OnInitializeAnimInstance(InProxy, InAnimInstance);
}