// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.

#include "StepVrStream.h"
#include "StepIkClientCpp.h"
#include "LocalFace.h"
#include "Animation/AnimInstanceProxy.h"

namespace StepMocapServers
{
	static bool IsConnected = false;
	static TMap<uint32, TSharedPtr<FStepMocapStream>> AllStreams;

	TWeakPtr<FStepMocapStream> GetStream(const FMocapServerInfo& InServerInfo);
	void ReturnStream(const FMocapServerInfo& InServerInfo);

	uint32 GetServerID(const FMocapServerInfo& InServerInfo);
}

uint32 StepMocapServers::GetServerID(const FMocapServerInfo& InServerInfo)
{
	FString ServerString = InServerInfo.ServerIP + FString::FormatAsNumber(InServerInfo.ServerPort);
	uint32 ServerID = GetTypeHash(ServerString);
	return ServerID;
}
TWeakPtr<FStepMocapStream> StepMocapServers::GetStream(const FMocapServerInfo& InServerInfo)
{
	uint32 ServerID = StepMocapServers::GetServerID(InServerInfo);

	TWeakPtr<FStepMocapStream> TargetStream;

	auto Temp = AllStreams.Find(ServerID);
	if (Temp == nullptr)
	{
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
		UE_LOG(LogTemp, Warning, TEXT("StepMocapGetStreamFaild : ServerIP:%s , ServerPort:%d"), *InServerInfo.ServerIP, InServerInfo.ServerPort);
	}

	return TargetStream;
}


void ConvertToUE(transform* InData, TArray<FTransform>& OutData)
{
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
			UE_LOG(LogTemp, Warning, TEXT("%f--%f--%f--%f"), TempQuat.X, TempQuat.Y, TempQuat.Z, TempQuat.W);
			OutData.Empty();
			break;
		}

		TempData.SetLocation(TempVec);
		TempData.SetRotation(TempQuat);
	}
}
void ConvertToUE(GCWT::transform* InData, TArray<FRotator>& OutData)
{
	OutData.Init(FRotator::ZeroRotator, STEPHANDBONESNUMS);

	FVector TempVec;
	FQuat TempQuat;

	static const FQuat QBA = FQuat::FQuat(FVector(0, 0, 1), PI);
	static const FQuat QTE = FQuat::FQuat(FVector(0, 0, 1), -PI / 2.0).Inverse()*FQuat::FQuat(FVector(1, 0, 0), -PI / 2.0).Inverse();
	for (int32 i = 0; i < STEPHANDBONESNUMS; i++)
	{
		FRotator& TempData = OutData[i];

		TempQuat.X = InData[i].Rotation.x;
		TempQuat.Y = InData[i].Rotation.y;
		TempQuat.Z = InData[i].Rotation.z;
		TempQuat.W = InData[i].Rotation.w;

		if (i == 0)
		{
			TempQuat = QBA.Inverse()*TempQuat *QBA;
		}
		TempQuat = TempQuat * QBA;
		TempQuat = QTE * TempQuat;
		//TempQuat = QBA.Inverse()*TempQuat *QBA;

		if (!TempQuat.IsNormalized())
		{
			UE_LOG(LogTemp, Warning, TEXT("%f--%f--%f--%f"), TempQuat.X, TempQuat.Y, TempQuat.Z, TempQuat.W);
			OutData.Empty();
			break;
		}

		TempData = TempQuat.Rotator();
	}
}
void StepMocapServers::ReturnStream(const FMocapServerInfo& InServerInfo)
{
	uint32 ServerID = StepMocapServers::GetServerID(InServerInfo);
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
	UE4BoneIndices.SetNumUninitialized(STEPBONESNUMS);
	UE4HandBoneIndices.SetNumUninitialized(STEPHANDBONESNUMS);
}

FStepDataToSkeletonBinding::~FStepDataToSkeletonBinding()
{
	if (StepMpcapStream.IsValid())
	{
		StepMocapServers::ReturnStream(StepMpcapStream.Pin()->GetServerInfo());
		StepMpcapStream.Reset();
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
	if (StepMpcapStream.Pin()->GetServerInfo().IsEqual(InServerInfo))
	{
		return true;
	}

	//释放并创建新的Steam
	StepMocapServers::ReturnStream(InServerInfo);
	StepMpcapStream.Reset();
	StepMpcapStream = StepMocapServers::GetStream(InServerInfo);

	return StepMpcapStream.IsValid();
}

bool FStepDataToSkeletonBinding::IsConnected()
{
	return StepMpcapStream.IsValid() && StepMpcapStream.Pin()->IsConnected();
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
			UE_LOG(LogTemp,Error,TEXT("StepVrMocap Error Bind %s"),*BoneName);
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
			UE_LOG(LogTemp, Error, TEXT("StepVrMocap Error Bind %s"), *BoneName);
		}
	}

	mHandBound = UE4HandBoneIndices.Num() == STEPHANDBONESNUMS;

	return mHandBound;
}

bool FStepDataToSkeletonBinding::IsHandBound()
{
	return mHandBound;
}

bool FStepDataToSkeletonBinding::UpdateHandFrameData(TArray<FRotator>& outPose)
{
	GCWT::transform Gtransform[STEPHANDBONESNUMS];
	TArray<FRotator> UEMocapData;

	//FStepMocapStream::GetLocalGlove()->GetGloveData(Gtransform, std::string(TCHAR_TO_UTF8(*mIpAddress)));
	//FStepMocapStream::GetLocalGlove()->GetGloveData(Gtransform, "192.168.50.102");
	FStepMocapStream::GetLocalGlove()->GetGloveData(Gtransform, TCHAR_TO_UTF8(*mIpAddress));
	ConvertToUE(Gtransform, UEMocapData);

	if (UEMocapData.Num() != STEPHANDBONESNUMS)
	{
		return false;
	}

	outPose.Empty(STEPHANDBONESNUMS);
	for (int32 i = 0; i < STEPHANDBONESNUMS; i++)
	{
		outPose.Add(UEMocapData[i]);
	}

	return true;
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

bool FStepDataToSkeletonBinding::IsBound()
{
	return mBound;
}

bool FStepDataToSkeletonBinding::UpdateFrameData(TArray<FTransform>& outPose)
{
	if (!StepMpcapStream.IsValid())
	{
		return false;
	}

	StepMpcapStream.Pin()->GetBonesTransform(outPose);

	return true;
}

FStepMocapStream::FStepMocapStream()
{
	ReferenceCount = 0;
	CacheFrameData.SetNumUninitialized(STEPBONESNUMS);
}

FStepMocapStream::~FStepMocapStream()
{
	UE_LOG(LogTemp, Log, TEXT("StepVrMocap StepMocapStream Delete"));
	if (StepVrClient.IsValid())
	{
		StepVrClient->StopData();
	}
}

GCWT::LocalFace* GLocalFace = nullptr;
GCWT::LocalFace* FStepMocapStream::GetLocalFace()
{
	if (GLocalFace == nullptr)
	{
		GLocalFace = new GCWT::LocalFace();
	}
	return GLocalFace;
}
GCWT::LocalGlove* GLocalGlove = nullptr;
GCWT::LocalGlove* FStepMocapStream::GetLocalGlove()
{
	if (GLocalGlove == nullptr)
	{
		GLocalGlove = new GCWT::LocalGlove();
	}
	return GLocalGlove;
}

bool FStepMocapStream::ReleaseReference()
{
	--ReferenceCount;
	UE_LOG(LogTemp,Log,TEXT("StepVrMocap ReferenceCount : %d  IPAddress : %s"), ReferenceCount, *UsedServerInfo.ServerIP);
	return ReferenceCount <= 0;
}

void FStepMocapStream::NeedReference()
{
	++ReferenceCount;
	UE_LOG(LogTemp, Log, TEXT("StepVrMocap ReferenceCount : %d  IPAddress : %s"), ReferenceCount, *UsedServerInfo.ServerIP);
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

	StepVrClient = MakeShareable(new StepIK_Client());

	ConnectToServer();
}

const FMocapServerInfo& FStepMocapStream::GetServerInfo()
{
	return UsedServerInfo;
}

void FStepMocapStream::GetBonesTransform(TArray<FTransform>& BonesData)
{
	if (STEPBONESNUMS != StepBonesID.Num())
	{
		return;
	}

	if (GFrameCounter == PreCacheFrame)
	{
		BonesData.Empty(CacheFrameData.Num());
	}
	else
	{
		static transform StepVrMocapData[STEPBONESNUMS];
		static TArray<FTransform> UEMocapData;

		//帧号
		PreCacheFrame = GFrameCounter;
		StepVrClient->getData((transform*)StepVrMocapData);
		ConvertToUE(StepVrMocapData, UEMocapData);

		CacheFrameData.Empty();
		FTransform TeampData;
		for (int32 i = 0 ; i < UEMocapData.Num(); i++)
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

			CacheFrameData.Add(TeampData);
		}
	}

	BonesData = CacheFrameData;
}

bool FStepMocapStream::ConnectToServer()
{
	if (!StepVrClient.IsValid())
	{
		return false;
	}

	if (bIsConnected)
	{
		return bIsConnected;
	}

	do 
	{
		int32 Flag = StepVrClient->Connect(TCHAR_TO_ANSI(*UsedServerInfo.ServerIP), UsedServerInfo.ServerPort);

		if (StepVrClient->GetServerStatus() > 0)
		{
			break;
		}

		bIsConnected = true;
	} while (0);


	if (bIsConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("StepVrMocap Connect: %s Success"), *UsedServerInfo.ServerIP);
		StepVrClient->startData();
	}
	else
	{
		//错误
		UE_LOG(LogTemp, Warning, TEXT("StepVrMocap Connect Error : %s"), *UsedServerInfo.ServerIP);
	}

	return bIsConnected;
}
