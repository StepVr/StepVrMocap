// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.

#include "StepVrStream.h"
#include "StepIkClientCpp.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/MorphTarget.h"
#include "Misc/CoreDelegates.h"




static transform StepVrMocapData[STEPBONESNUMS];
static TArray<FTransform> UEMocapData;


transform GTR;

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


void ConvertToUE(transform* InData, TArray<FTransform>& OutData)
{

	GTR = InData[9];

	OutData.Init(FTransform::Identity, STEPBONESNUMS);

	FVector TempVec;
	FQuat TempQuat;

	static const FQuat QBA = FQuat::FQuat(FVector(0, 0, 1), PI);
	static const FQuat QTE = FQuat::FQuat(FVector(0, 0, 1), -PI / 2.0).Inverse()*FQuat::FQuat(FVector(1, 0, 0), -PI / 2.0).Inverse();
	for (int32 i = 0; i < STEPBONESNUMS; i++)
	{
		FTransform& TempData = OutData[i];

		TempVec.X = InData[i].Location.z * 100;
		TempVec.Y = InData[i].Location.x * 100;
		TempVec.Z = InData[i].Location.y * 100;

		TempQuat.X = InData[i].Rotation.x;
		TempQuat.Y = InData[i].Rotation.y;
		TempQuat.Z = InData[i].Rotation.z;
		TempQuat.W = InData[i].Rotation.w;

		TempQuat = TempQuat * QBA;
		TempQuat = QTE * TempQuat;

		if (!TempQuat.IsNormalized())
		{
			FString Message = FString::Printf(TEXT("%f--%f--%f--%f"), TempQuat.X, TempQuat.Y, TempQuat.Z, TempQuat.W);
			ShowMessage(Message);
			OutData.Empty();
			break;
		}

		TempData.SetLocation(TempVec);
		TempData.SetRotation(TempQuat);
	}
}
void ConvertToUE(V4* InData, TArray<FRotator>& OutData)
{
	OutData.Init(FRotator::ZeroRotator, STEPHANDBONESNUMS);

	FQuat TempQuat;
	static const FQuat QBA = FQuat::FQuat(FVector(0, 0, 1), PI);
	static const FQuat QTE = FQuat::FQuat(FVector(0, 0, 1), -PI / 2.0).Inverse()*FQuat::FQuat(FVector(1, 0, 0), -PI / 2.0).Inverse();
	for (int32 i = 0; i < STEPHANDBONESNUMS; i++)
	{
		TempQuat.X = InData[i].x;
		TempQuat.Y = InData[i].y;
		TempQuat.Z = InData[i].z;
		TempQuat.W = InData[i].w;
		
		TempQuat = TempQuat*QBA;
		TempQuat = QTE * TempQuat;

		if (!TempQuat.IsNormalized())
		{
			FString Message = FString::Printf(TEXT("%f--%f--%f--%f"), TempQuat.X, TempQuat.Y, TempQuat.Z, TempQuat.W);
			ShowMessage(Message);
			OutData.Empty();
			break;
		}

		OutData[i] = TempQuat.Rotator();
	}
}



// Default constructor.
FStepDataToSkeletonBinding::FStepDataToSkeletonBinding()
{
	UE4BoneIndices.SetNumUninitialized(STEPBONESNUMS);
	UE4HandBoneIndices.SetNumUninitialized(STEPHANDBONESNUMS);
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
	mIpAddress = InServerInfo.ServerIP;
	if (!StepMpcapStream.IsValid())
	{
		//创建新的Steam
		StepMpcapStream = StepMocapServers::GetStream(InServerInfo);
		return StepMpcapStream.IsValid();
	}

	//与当前的是否相同
	if (StepMpcapStream->GetServerInfo().IsEqual(InServerInfo))
	{
		return true;
	}

	//释放并创建新的Steam
	StepMocapServers::ReturnStream(StepMpcapStream.ToSharedRef());
	StepMpcapStream = StepMocapServers::GetStream(InServerInfo);

	return StepMpcapStream.IsValid();
}

bool FStepDataToSkeletonBinding::IsConnected()
{
	return StepMpcapStream.IsValid() && StepMpcapStream->IsConnected();
}

// Bind the given Strean to the given skeleton and store the result.
bool FStepDataToSkeletonBinding::BindToSkeleton(FAnimInstanceProxy* AnimInstanceProxy, TMap<FString, FBoneReference>& BoneReferences)
{
	mBound = false;

	USkeleton* Skeleton = AnimInstanceProxy->GetSkeleton();
	if (Skeleton == nullptr)
	{
		return mBound;
	}

	UE4BoneIndices.Empty(STEPBONESNUMS);
	int32 ue4BoneIndex = INDEX_NONE;

	for (auto& BoneName :StepBoneNames)
	{
		FBoneReference* Ref = BoneReferences.Find(BoneName);
		if (Ref == nullptr)
		{
			continue;
		}

		Ref->Initialize(Skeleton);

		if (Ref->HasValidSetup())
		{
			UE4BoneIndices.Add(Ref->BoneIndex);
		}
		else
		{
			FString Message = FString::Printf(TEXT("StepVrMocap Error Bind %s"), *BoneName);
			ShowMessage(Message);
		}
	}

	mBound = UE4BoneIndices.Num() == STEPBONESNUMS;

	return mBound;
}

bool FStepDataToSkeletonBinding::BindToHandSkeleton(FAnimInstanceProxy* AnimInstanceProxy, TMap<FString, FBoneReference>& BoneReferences)
{
	mHandBound = false;

	USkeleton* Skeleton = AnimInstanceProxy->GetSkeleton();
	if (Skeleton == nullptr)
	{
		return mHandBound;
	}

	UE4HandBoneIndices.Empty(STEPHANDBONESNUMS);
	int32 ue4BoneIndex = INDEX_NONE;

	for (auto& BoneName : StepHandBoneNames)
	{
		FBoneReference* Ref = BoneReferences.Find(BoneName);
		if (Ref == nullptr)
		{
			continue;
		}

		Ref->Initialize(Skeleton);

		if (Ref->HasValidSetup())
		{
			UE4HandBoneIndices.Add(Ref->BoneIndex);
		}
		else
		{
			FString Message = FString::Printf(TEXT("StepVrMocap Error Bind %s"), *BoneName);
			ShowMessage(Message);
		}
	}

	mHandBound = UE4HandBoneIndices.Num() == STEPHANDBONESNUMS;

	return mHandBound;
}

bool FStepDataToSkeletonBinding::IsHandBound()
{
	return mHandBound;
}

float FStepDataToSkeletonBinding::GetFigureScale()
{
	return 1.f;
}

// Access the UE4 bone index given the bone def index.
int32 FStepDataToSkeletonBinding::GetUE4BoneIndex(int32 boneDefIndex) const
{
	if (UE4BoneIndices.IsValidIndex(boneDefIndex))
		return UE4BoneIndices[boneDefIndex];
	return INDEX_NONE;
}

int32 FStepDataToSkeletonBinding::GetUE4HandBoneIndex(int32 SegmentIndex) const
{
	if (UE4HandBoneIndices.IsValidIndex(SegmentIndex))
		return UE4HandBoneIndices[SegmentIndex];
	return INDEX_NONE;
}


bool FStepDataToSkeletonBinding::BindToFaceMorghTarget(FAnimInstanceProxy* AnimInstanceProxy, TArray<FString>& MorghTarget)
{
	mFaceBound = false;
	MorghTarget.Empty();

	USkeletalMeshComponent* Cpmponent = AnimInstanceProxy->GetSkelMeshComponent();
	if (Cpmponent == nullptr)
	{
		return mFaceBound;
	}

	if (Cpmponent->SkeletalMesh == nullptr)
	{
		return mFaceBound;
	}

	for (int32 i = 0; i < Cpmponent->SkeletalMesh->MorphTargets.Num(); i++)
	{
		FString MorghName = Cpmponent->SkeletalMesh->MorphTargets[i]->GetName();
		MorghTarget.Add(MorghName);
	}

	mFaceBound = MorghTarget.IsValidIndex(0);
	return mFaceBound;
}

bool FStepDataToSkeletonBinding::UpdateFaceFrameData(TMap<FString, float>& outPose)
{
	if (!StepMpcapStream.IsValid())
	{
		return false;
	}

	StepMpcapStream->GetBonesTransform_Face(outPose);

	return true;
}

bool FStepDataToSkeletonBinding::IsFaceBound()
{
	return mFaceBound;
}

bool FStepDataToSkeletonBinding::IsBodyBound()
{
	return mBound;
}

bool FStepDataToSkeletonBinding::UpdateBodyFrameData(TArray<FTransform>& outPose)
{
	if (!StepMpcapStream.IsValid())
	{
		return false;
	}

	StepMpcapStream->GetBonesTransform_Body(outPose);

	return true;
}
bool FStepDataToSkeletonBinding::UpdateHandFrameData(TArray<FRotator>& outPose)
{
	if (!StepMpcapStream.IsValid())
	{
		return false;
	}

	StepMpcapStream->GetBonesTransform_Hand(outPose);

	return true;
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

	ConnectToServer();

	//注册更新
	EngineBeginHandle = FCoreDelegates::OnBeginFrame.AddRaw(this, &FStepMocapStream::EngineBegineFrame);
}

const FMocapServerInfo& FStepMocapStream::GetServerInfo()
{
	return UsedServerInfo;
}

void FStepMocapStream::GetBonesTransform_Body(TArray<FTransform>& BonesData)
{
	BonesData.Empty();

	if (CacheBodyFrameData.Num() > 0)
	{
		BonesData = CacheBodyFrameData;
	}
}

void FStepMocapStream::GetBonesTransform_Hand(TArray<FRotator>& BonesData)
{
	BonesData.Empty();

	if (CacheHandFrameData.Num() > 0)
	{
		BonesData = CacheHandFrameData;
	}
}

void FStepMocapStream::GetBonesTransform_Face(TMap<FString, float>& BonesData)
{
	BonesData.Empty();

	if (CacheFaceFrameData.Num() > 0)
	{
		BonesData = CacheFaceFrameData;
	}
}

void FStepMocapStream::ConnectToServer()
{
	//创建连接
	StepVrClient = MakeShareable(new StepIK_Client());
	

	int32 Flag = StepVrClient->Connect(TCHAR_TO_ANSI(*UsedServerInfo.ServerIP), UsedServerInfo.ServerPort);
	bClientConnected = Flag == 0;

	if (bClientConnected)
	{
		FString Message = FString::Printf(TEXT("StepVrMocap Connect %s Success"), *UsedServerInfo.ServerIP);
		ShowMessage(Message);
	}
	else
	{
		FString Message = FString::Printf(TEXT("StepVrMocap Connect %s Error"), *UsedServerInfo.ServerIP);
		ShowMessage(Message);
		return;
	}

	FString Message = "";

	//连接动捕
	do 
	{
		if (StepVrClient->GetServerStatus() > 0)
		{
			Message = TEXT("Mocap ServerStatus Error");
			break;
		}

		StepVrClient->startData();
		

		if (StepVrClient->HasBodyData())
		{
			bBodyConnected = true;
			Message = TEXT("Mocap Data Success");
		}
		else
		{
			Message = TEXT("Mocap Data Error");
		}

		bBodyConnected = true;
	} while (0);
	ShowMessage(Message);

	//连接手套
	do 
	{
		StepVrClient->GloEnable(true);
		StepVrClient->GloSetDir(135);
		StepVrClient->GloSetRotType(1);

		if (StepVrClient->HasGloveData())
		{
			bHandConnected = true;
			Message = TEXT("Glove Data Success");
		}
		else
		{
			Message = TEXT("Glove Data Error");
		}
		bHandConnected = true;
	} while (0);
	ShowMessage(Message);

	//连接面捕
	do 
	{
		if (StepVrClient->HasFaceData())
		{
			bFaceConnected = true;
			Message = TEXT("Face Data Success");
		}
		else
		{
			Message = TEXT("Face Data Error");
		}
		bFaceConnected = true;
	} while (0);
	ShowMessage(Message);
}
void FStepMocapStream::DisconnectToServer()
{
	bClientConnected = false;
	bBodyConnected = false;
	bHandConnected = false;
	bFaceConnected = false;

	if (StepVrClient.IsValid())
	{
		StepVrClient->StopData();
		StepVrClient->GloEnable(false);
		StepVrClient.Reset();
	}

	FString Message = FString::Printf(TEXT("StepMocapStream Delete : %s"), *UsedServerInfo.ServerIP);
	ShowMessage(Message);

	FCoreDelegates::OnBeginFrame.Remove(EngineBeginHandle);
}
void FStepMocapStream::EngineBegineFrame()
{
	if (!bClientConnected)
	{
		return;
	}

	//动捕数据
	if (bBodyConnected)
	{
		StepVrClient->getData((transform*)StepVrMocapData);
		ConvertToUE(StepVrMocapData, UEMocapData);

		CacheBodyFrameData.Empty();
		FTransform TeampData;
		for (int32 i = 0; i < UEMocapData.Num(); i++)
		{
			if (i >= 1)
			{
				TeampData.SetLocation(UEMocapData[StepBonesID[i]].GetRotation().Inverse() * (UEMocapData[i].GetLocation() - UEMocapData[StepBonesID[i]].GetLocation()));
				TeampData.SetRotation(UEMocapData[StepBonesID[i]].GetRotation().Inverse() * UEMocapData[i].GetRotation());
			}
			else
			{
				TeampData = UEMocapData[i];
			}

			CacheBodyFrameData.Add(TeampData);
		}
	}

	//手套数据
	do 
	{
		if (!bHandConnected)
		{
			break;
		}

		TArray<FRotator> UEMocapDataGlobal;
		V4 GRotators[STEPHANDBONESNUMS];
		CacheHandFrameData.Empty();

		StepVrClient->GetGloveData(GRotators);
		ConvertToUE(GRotators, UEMocapDataGlobal);

		if (UEMocapDataGlobal.Num() != STEPHANDBONESNUMS)
		{
			break;
		}

		//转化为Local
		FQuat TeampData = FQuat::Identity;
		for (int32 i = 0; i < UEMocapDataGlobal.Num(); i++)
		{
			TeampData = FQuat::Identity;
			if (StepHandBonesID[i] < 100)
			{
				TeampData = (UEMocapDataGlobal[StepHandBonesID[i]].Quaternion().Inverse() * UEMocapDataGlobal[i].Quaternion());
			}

			else if (StepHandBonesID[i] == 100 && UEMocapData.IsValidIndex(9))
			{
				//左手
				FQuat tmm0 = UEMocapDataGlobal[0].Quaternion();
				FQuat tmm1 = UEMocapDataGlobal[16].Quaternion();
				TeampData = (UEMocapData[9].GetRotation().Inverse() * UEMocapDataGlobal[0].Quaternion());
			}
			else if (StepHandBonesID[i] == 101 && UEMocapData.IsValidIndex(13))
			{
				//右手
				TeampData = (UEMocapData[13].GetRotation().Inverse() * UEMocapDataGlobal[16].Quaternion());
			}
			CacheHandFrameData.Add(FRotator(TeampData));
		}
	} while (0);



	if (bFaceConnected)
	{
		static float GFaceData[200];
		int32 length;
		StepVrClient->GetFaceData(GFaceData, length);

		CacheFaceFrameData.Empty(length);

		static UEnum* GRootEnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("FStepFaceMorghs"), true);
		for (int32 i = 0; i < length; i++)
		{
			FString CurShooterDataStr(GRootEnumPtr->GetNameByValue(i).ToString());
			CacheFaceFrameData.Add(CurShooterDataStr, GFaceData[i]);
		}
	}
}


