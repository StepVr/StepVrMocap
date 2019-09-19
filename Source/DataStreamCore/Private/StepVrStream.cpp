// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.

#include "StepVrStream.h"
#include "StepVrGlobal.h"
#include "StepVrServerModule.h"
#include "StepVrDataServer.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/MorphTarget.h"
#include "Misc/CoreDelegates.h"
#include "Kismet/KismetMathLibrary.h"


namespace StepMocapServers
{
	static bool IsConnected = false;
	static TMap<uint32, TSharedPtr<FStepMocapStream>> AllStreams;

	TSharedRef<FStepMocapStream> GetStream(const FMocapServerInfo& InServerInfo);
	void ReturnStream(TSharedRef<FStepMocapStream> StepMocapStream);

	uint32 GetServerID(const FMocapServerInfo& InServerInfo);
}

uint32 StepMocapServers::GetServerID(const FMocapServerInfo& InServerInfo)
{
	FString ServerString = InServerInfo.ServerIP + FString::FormatAsNumber(InServerInfo.ServerPort);
	uint32 ServerID = GetTypeHash(ServerString);
	return ServerID;
}


TSharedRef<FStepMocapStream> StepMocapServers::GetStream(const FMocapServerInfo& InServerInfo)
{
	uint32 ServerID = StepMocapServers::GetServerID(InServerInfo);

	TWeakPtr<FStepMocapStream> TargetStream;

	auto Temp = AllStreams.Find(ServerID);
	if (Temp == nullptr || !(*Temp).IsValid())
	{
		//移除原有的
		AllStreams.Remove(ServerID);

		TSharedPtr<FStepMocapStream> NewStream = MakeShareable(new FStepMocapStream());
		NewStream->SetServerInfo(InServerInfo);
		TargetStream = NewStream;
		AllStreams.Add(ServerID, NewStream);
	}
	else
	{
		TargetStream = *Temp;
	}

	if (TargetStream.IsValid())
	{
		TargetStream.Pin()->NeedReference();
	}
	else
	{
		FString Message = FString::Printf(TEXT("StepMocapGetStreamFaild : ServerIP:%s , ServerPort:%d"), *InServerInfo.ServerIP, InServerInfo.ServerPort);
		ShowMessage(Message);
	}

	return TargetStream.Pin().ToSharedRef();
}

void StepMocapServers::ReturnStream(TSharedRef<FStepMocapStream> StepMocapStream)
{
	uint32 ServerID = StepMocapServers::GetServerID(StepMocapStream->GetServerInfo());
	auto Temp = AllStreams.Find(ServerID);

	bool NeedRelease = (Temp != nullptr) && (*Temp)->ReleaseReference();
	if (NeedRelease)
	{
		AllStreams.Remove(ServerID);
	}
}






// Default constructor.
FStepDataToSkeletonBinding::FStepDataToSkeletonBinding()
{
}

FStepDataToSkeletonBinding::~FStepDataToSkeletonBinding()
{
	if (StepMpcapStream.IsValid())
	{
		StepMocapServers::ReturnStream(StepMpcapStream.ToSharedRef());
	}
}


bool FStepDataToSkeletonBinding::ConnectToServer(const FMocapServerInfo& InServerInfo)
{
	if (InServerInfo.IsEmpty())
	{
		return false;
	}

	//远端模拟
	if (InServerInfo.StepControllState == Remote_Replicate_Y)
	{
		CacheServerInfo = InServerInfo;

		if (StepMpcapStream.IsValid())
		{
			StepMocapServers::ReturnStream(StepMpcapStream.ToSharedRef());
			StepMpcapStream.Reset();
		}
	}
	else
	{
		if (CacheServerInfo.IsEqual(InServerInfo))
		{
			return StepMpcapStream.IsValid() && StepMpcapStream->IsConnected();
		}

		CacheServerInfo = InServerInfo;

		//创建新的Steam
		if (!StepMpcapStream.IsValid())
		{
			StepMpcapStream = StepMocapServers::GetStream(CacheServerInfo);
			return StepMpcapStream.IsValid();
		}

		//与当前的是否相同
		if (StepMpcapStream->GetServerInfo().IsEqual(CacheServerInfo))
		{
			return true;
		}

		//释放并创建新的Steam
		StepMocapServers::ReturnStream(StepMpcapStream.ToSharedRef());
		StepMpcapStream = StepMocapServers::GetStream(CacheServerInfo);

		return StepMpcapStream.IsValid();
	}

	return true;
}

void FStepDataToSkeletonBinding::BindToSkeleton(FAnimInstanceProxy* AnimInstanceProxy, BoneMappings& BodyBoneReferences, BoneMappings& HandBoneReferences)
{
	USkeleton* Skeleton = AnimInstanceProxy->GetSkeleton();
	if (Skeleton == nullptr)
	{
		return;
	}

	UE4BoneIndices.Empty();
	FMapBone CurMapBone;

	/**
	 * 身体骨骼
	 */
	for (int32 Index = 0; Index < StepBoneNames.Num(); Index++)
	{
		FBoneReference* Ref = BodyBoneReferences.Find(StepBoneNames[Index]);
		if (Ref == nullptr)
		{
			continue;
		}

		Ref->Initialize(Skeleton);

		if (Ref->HasValidSetup() && Ref->BoneIndex != 0)
		{
			CurMapBone.MapBoneType = EMapBoneType::Bone_Body;
			CurMapBone.StepBoneIndex = Index;
			CurMapBone.UeBoneIndex = Ref->BoneIndex;
			UE4BoneIndices.Add(CurMapBone);
		}
	}


	/**
	* 手部骨骼
	*/
	for (int32 Index = 0; Index < StepHandBoneNames.Num(); Index++)
	{
		FBoneReference* Ref = HandBoneReferences.Find(StepHandBoneNames[Index]);
		if (Ref == nullptr)
		{
			continue;
		}

		Ref->Initialize(Skeleton);

		if (Ref->HasValidSetup())
		{
			CurMapBone.MapBoneType = EMapBoneType::Bone_Hand;
			CurMapBone.StepBoneIndex = Index;
			CurMapBone.UeBoneIndex = Ref->BoneIndex;
			UE4BoneIndices.Add(CurMapBone);
		}
	}


	UE4BoneIndices.Sort([](const FMapBone& data1, const FMapBone&data2)
	{
		return data1.UeBoneIndex < data2.UeBoneIndex;
	});

	UE4NeedUpdateBones.Empty();
	UE4NeedUpdateBones.AddUnique(0);
	const FBoneContainer& BoneContainer = AnimInstanceProxy->GetRequiredBones();
	for (int Index = 0; Index < UE4BoneIndices.Num(); Index++)
	{
		//当前节点
		int32 CurIdnex = UE4BoneIndices[Index].UeBoneIndex;
		UE4NeedUpdateBones.AddUnique(CurIdnex);
	
		//父节点
		while ((CurIdnex = BoneContainer.GetParentBoneIndex(CurIdnex)) > 0)
		{
			if (UE4NeedUpdateBones.Find(CurIdnex) == INDEX_NONE)
			{
				UE4NeedUpdateBones.Add(CurIdnex);
			}
			else
			{
				break;
			}
		}
	}

	UE4NeedUpdateBones.Sort([](const int32& Data1, const int32& Data2)
	{
		return Data1 < Data2;
	});
	
}

// Access the UE4 bone index given the bone def index.
const FStepDataToSkeletonBinding::FMapBone& FStepDataToSkeletonBinding::GetUE4BoneIndex(int32 boneDefIndex) const
{
	if (UE4BoneIndices.IsValidIndex(boneDefIndex))
	{
		return UE4BoneIndices[boneDefIndex];
	}

	static FMapBone GTempData;
	return GTempData;
}

const TArray<FStepDataToSkeletonBinding::FMapBone>& FStepDataToSkeletonBinding::GetUE4Bones()
{
	return UE4BoneIndices;
}

void FStepDataToSkeletonBinding::BindToFaceMorghTarget(FAnimInstanceProxy* AnimInstanceProxy, TMap<FString, FString>& MorghTarget)
{
	mFaceBound = false;
	UE4FaceData.Empty();

	if (!CacheServerInfo.EnableFace)
	{
		return;
	}

	USkeletalMeshComponent* Cpmponent = AnimInstanceProxy->GetSkelMeshComponent();
	if (Cpmponent == nullptr || Cpmponent->SkeletalMesh == nullptr)
	{
		return;
	}

	FStepDataToSkeletonBinding::FMorphData CurData;
	for (auto& Temp : MorghTarget)
	{
		auto Finder = Cpmponent->SkeletalMesh->MorphTargetIndexMap.Find(FName(*Temp.Value));
		if (Finder)
		{
			CurData.MorphValue = 0.f;
			CurData.StepMarphName = FName(*Temp.Key);
			CurData.UE4MarphName = FName(*Temp.Value);
			UE4FaceData.Add(CurData);
		}
	}

	mFaceBound = UE4FaceData.Num() > 0;
}

void FStepDataToSkeletonBinding::UpdateFaceFrameData()
{
	if (!CacheServerInfo.EnableFace)
	{
		return;
	}

	if (!StepMpcapStream.IsValid())
	{
		return;
	}

	auto FacaeData = StepMpcapStream->GetBonesTransform_Face();
	if (FacaeData.Num() == 0)
	{
		return;
	}

	for (auto& FaceBind : UE4FaceData)
	{
		auto FindPtr = FacaeData.Find(FaceBind.StepMarphName.ToString());
		if (FindPtr)
		{
			FaceBind.MorphValue = *FindPtr;
		}
	}
}

bool FStepDataToSkeletonBinding::IsFaceBound()
{
	return mFaceBound;
}


const TArray<FStepDataToSkeletonBinding::FMorphData>& FStepDataToSkeletonBinding::GetUE4FaceData()
{
	return UE4FaceData;
}

const FVector& FStepDataToSkeletonBinding::GetSkeletonScale()
{
	return SkeletonScale;
}

void FStepDataToSkeletonBinding::UpdateSkeletonFrameData()
{
	if (CacheServerInfo.StepControllState == Remote_Replicate_Y)
	{
		TArray<FTransform> SkeletonData;
		{
			FScopeLock Lock(&GReplicateSkeletonCS);
			auto FindData = GReplicateSkeletonRT.Find(CacheServerInfo.AddrValue);
			if (FindData == nullptr)
			{
				return;
			}

			SkeletonData = *FindData;
		}

		if (SkeletonData.Num() != STEPBONESNUMS)
		{
			return;
		}

		for (auto& Temp : UE4BoneIndices)
		{
			if (Temp.MapBoneType == EMapBoneType::Bone_Body)
			{
				//FTransform Tlerp = UKismetMathLibrary::TLerp(Temp.BoneData, SkeletonData[Temp.StepBoneIndex],CurFrame);
				//Temp.BoneData = Tlerp;
				Temp.BoneData = SkeletonData[Temp.StepBoneIndex];
			}
		}
	}
	else
	{
		if (!StepMpcapStream.IsValid())
		{
			return;
		}

		auto BodyData = StepMpcapStream->GetBonesTransform_Body();
		bool bBodyData = BodyData.Num() == STEPBONESNUMS;

		auto HandData = StepMpcapStream->GetBonesTransform_Hand();
		bool bHandData = CacheServerInfo.EnableHand && HandData.Num() == STEPHANDBONESNUMS;

		//动捕缩放
		if (bBodyData)
		{
			SkeletonScale = BodyData[0].GetScale3D();
		}

		//动捕数据
		for (auto& Temp : UE4BoneIndices)
		{
			switch (Temp.MapBoneType)
			{
			case EMapBoneType::Bone_Body:
			{
				if (bBodyData)
				{
					Temp.BoneData = BodyData[Temp.StepBoneIndex];
				}
			}
			break;
			case EMapBoneType::Bone_Hand:
			{
				if (bHandData)
				{
					Temp.BoneData = FTransform(HandData[Temp.StepBoneIndex]);
				}
			}
			break;
			}
		}
	}
}

const TArray<int32>& FStepDataToSkeletonBinding::GetUE4NeedUpdateBones()
{
	return UE4NeedUpdateBones;
}

FStepMocapStream::FStepMocapStream()
{

}

FStepMocapStream::~FStepMocapStream()
{
	DisconnectToServer();
}

bool FStepMocapStream::ReleaseReference()
{
	--ReferenceCount;
	return ReferenceCount <= 0;
}

void FStepMocapStream::NeedReference()
{
	++ReferenceCount;
}

void FStepMocapStream::SetServerInfo(const FMocapServerInfo& ServerInfo)
{
	if (ServerInfo.IsEmpty())
	{
		return;
	}

	if (ServerInfo.IsEqual(UsedServerInfo))
	{
		return;
	}

	//初始化服务
	UsedServerInfo = ServerInfo;

	//初始化数据池
	ReferenceCount = 0;

	/**
	 * 连接数据池，注册更新
	 */
	ConnectToServices();
}

const FMocapServerInfo& FStepMocapStream::GetServerInfo()
{
	return UsedServerInfo;
}

TArray<FTransform>& FStepMocapStream::GetBonesTransform_Body()
{
	return CacheBodyFrameData;
}

TArray<FRotator>& FStepMocapStream::GetBonesTransform_Hand()
{
	return CacheHandFrameData;
}

TMap<FString, float>& FStepMocapStream::GetBonesTransform_Face()
{
	return CacheFaceFrameData;
}

bool FStepMocapStream::IsConnected()
{
	return ServerConnect->IsConnected();
}

bool FStepMocapStream::IsBodyConnect()
{
	return ServerConnect->HasBodyData();
}

bool FStepMocapStream::IsHandConnect()
{
	return ServerConnect->HasHandData();
}

bool FStepMocapStream::IsFaceConnect()
{
	return ServerConnect->HasFaceData();
}

void FStepMocapStream::ConnectToServices()
{
	//创建连接
	if (!ServerConnect.IsValid())
	{
		ServerConnect = StepVrDataServer::CreateServerData();
		ServerConnect->ReceiveData.BindRaw(this, &FStepMocapStream::EngineBegineFrame);
	}

	ServerConnect->Connect2Server(TCHAR_TO_ANSI(*UsedServerInfo.ServerIP), UsedServerInfo.ServerPort);

	FString Message = FString::Printf(TEXT("StepVrMocap Connect %s"), *UsedServerInfo.ServerIP);
	ShowMessage(Message);
}
void FStepMocapStream::DisconnectToServer()
{
	if (ServerConnect.IsValid())
	{
		ServerConnect->DisConnect();
	}

	FString Message = FString::Printf(TEXT("StepMocapStream Delete : %s"), *UsedServerInfo.ServerIP);
	ShowMessage(Message);
}
void FStepMocapStream::EngineBegineFrame()
{
	if (!ServerConnect.IsValid() ||
		!ServerConnect->IsConnected())
	{
		return;
	}

	//动捕数据
	if (ServerConnect->HasBodyData())
	{
		ServerConnect->GetBodyData(CacheBodyFrameData);

		//同步数据
		if (UsedServerInfo.StepControllState == FStepControllState::Local_Replicate_Y && 
			STEPVR_SERVER_IsValid)
		{
			STEPVR_SERVER->StepMocapSendData(CacheBodyFrameData);
		}
	}

	//手套数据
	if (ServerConnect->HasHandData())
	{
		ServerConnect->GetHandData(CacheHandFrameData);
	}

	//面部数据
	if (ServerConnect->HasFaceData())
	{
		ServerConnect->GetFaceData(CacheFaceFrameData);
	}
}