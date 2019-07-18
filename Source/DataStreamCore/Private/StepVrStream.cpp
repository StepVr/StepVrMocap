// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.

#include "StepVrStream.h"
#include "StepIkClientCpp.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/MorphTarget.h"
#include "Misc/CoreDelegates.h"
#include "Kismet/GameplayStatics.h"

#if WITH_STEPMAGIC
#include "AJA_Corvid44.h"
#include "VirtualShootingDll.h"
#include "StepMagicThirdParty.h"
#include "StepMagicGlobal.h"
/**
 * 当前帧从StepMagic获取的数据
 */
static MFGValue GCur_IP_Frame;
#endif


/**
 * 当前帧从Service获取的数据
 */
static int32 GStepFaceLength = 0;
static float GStepFaceData[STEPFACEMORGHNUMS];
static V4 GStepHandData[STEPHANDBONESNUMS];
static transform GStepMocapData[STEPBONESNUMS];

/**
 * 动捕临时数据，手套子节点用
 */
static TArray<FTransform> UEMocapData;


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

	int32 TotalNums = 0;
	for (auto& BoneName : StepHandBoneNames)
	{
		FBoneReference* Ref = BoneReferences.Find(BoneName);
		if (Ref == nullptr)
		{
			UE4HandBoneIndices.Add(INDEX_NONE);
			continue;
		}

		Ref->Initialize(Skeleton);

		if (Ref->HasValidSetup())
		{
			UE4HandBoneIndices.Add(Ref->BoneIndex);
			TotalNums++;
		}
		else
		{
			FString Message = FString::Printf(TEXT("StepVrMocap Error Bind %s"), *BoneName);
			ShowMessage(Message);
		}
	}

	//两根base
	mHandBound = TotalNums >= (STEPHANDBONESNUMS - 2);

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
	//暂时不用
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

	/**
	 * 连接数据池，注册更新
	 */
#if WITH_STEPMAGIC
	ConnectToStepMagic();
	EngineBeginHandle = GStepMagicFrameBegin.AddRaw(this, &FStepMocapStream::EngineBegineFrame);
#else
	ConnectToServices();
	EngineBeginHandle = FCoreDelegates::OnBeginFrame.AddRaw(this, &FStepMocapStream::EngineBegineFrame);
#endif
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

bool FStepMocapStream::IsConnected()
{
	bool IsConnect = false;

#if WITH_STEPMAGIC
	IsConnect = true;
#else
	IsConnect = StepVrClient->IsConnected();
#endif
	return IsConnect;
}

bool FStepMocapStream::IsBodyConnect()
{
	bool IsConnect = false;
#if WITH_STEPMAGIC
	IsConnect = true;
#else
	IsConnect = StepVrClient->HasBodyData();
#endif
	return IsConnect;
}

bool FStepMocapStream::IsHandConnect()
{
	bool IsConnect = false;
#if WITH_STEPMAGIC
	IsConnect = true;
#else
	IsConnect = StepVrClient->HasGloveData();
#endif
	return IsConnect;
}

bool FStepMocapStream::IsFaceConnect()
{
	bool IsConnect = false;
#if WITH_STEPMAGIC
	IsConnect = true;
#else
	IsConnect = StepVrClient->HasFaceData();
#endif
	return IsConnect;
}

void FStepMocapStream::ConnectToServices()
{
	//创建连接
	if (!StepVrClient.IsValid())
	{
		StepVrClient = MakeShareable(new StepIK_Client());
	}

	StepVrClient->Connect(TCHAR_TO_ANSI(*UsedServerInfo.ServerIP), UsedServerInfo.ServerPort);
	
	//连接动捕
	StepVrClient->startData();

	//连接手套
	StepVrClient->GloEnable(true);

	FString Message = FString::Printf(TEXT("StepVrMocap Connect %s"), *UsedServerInfo.ServerIP);
	ShowMessage(Message);
}
void FStepMocapStream::ConnectToStepMagic()
{
}
void FStepMocapStream::DisconnectToServer()
{
#if WITH_STEPMAGIC
	GStepMagicFrameBegin.Remove(EngineBeginHandle);
#else
	FCoreDelegates::OnBeginFrame.Remove(EngineBeginHandle);
#endif

	if (StepVrClient.IsValid())
	{
		StepVrClient->StopData();
		StepVrClient->GloEnable(false);
		StepVrClient.Reset();
	}

	FString Message = FString::Printf(TEXT("StepMocapStream Delete : %s"), *UsedServerInfo.ServerIP);
	ShowMessage(Message);
}
void FStepMocapStream::EngineBegineFrame()
{
	if (CheckConnectToServer() == false)
	{
		return;
	}

	//动捕数据
	if (IsBodyConnect())
	{
		UpdateFrameData_Body();
		ConvertToUE(GStepMocapData, UEMocapData);

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
		if (!IsHandConnect())
		{
			break;
		}

		TArray<FRotator> UEMocapDataGlobal;
		UpdateFrameData_Hand();
		ConvertToUE(GStepHandData, UEMocapDataGlobal);

		CacheHandFrameData.Empty();
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



	if (IsFaceConnect())
	{
		UpdateFrameData_Face();

		//static UEnum* GRootEnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("FStepFaceMorghs"), true);
		//GRootEnumPtr->GetNameByValue(i).ToString()
		CacheFaceFrameData.Empty(GStepFaceLength);
		for (int32 i = 0; i < GStepFaceLength; i++)
		{
			if (StepFaceMorphTargets.IsValidIndex(i))
			{
				CacheFaceFrameData.Add(StepFaceMorphTargets[i], GStepFaceData[i]);
			}
		}
	}
}

#if WITH_STEPMAGIC
bool FStepMocapStream::UpdateStepMagicData()
{
	bool IsSuccess = false;
	CVirtualShootingDll* GVirtualShootingDll = nullptr;
	if (GVirtualShootingDll == nullptr)
	{
		FStepMagicThirdParty* StepMagicThirdParty = FAJA_Corvid44Module::GetStepMagicThirdParty();
		if (StepMagicThirdParty)
		{
			GVirtualShootingDll = StepMagicThirdParty->GetVirtualShooting();
		}
	}

	MultiTracker stMultiTracker;
	GVirtualShootingDll->GetMultiTracker(stMultiTracker);

	MFGKey Keys;
	Keys.strIP = std::string(TCHAR_TO_ANSI(*UsedServerInfo.ServerIP));
	Keys.nPort = UsedServerInfo.ServerPort;
	auto FindValue = stMultiTracker.mapMFG.find(Keys);
	if (FindValue != stMultiTracker.mapMFG.end())
	{
		GCur_IP_Frame = FindValue->second;
		IsSuccess = true;
	}

	return IsSuccess;
}
#endif

void FStepMocapStream::UpdateFrameData_Body()
{
#if WITH_STEPMAGIC
	FMemory::Memcpy(GStepMocapData, GCur_IP_Frame.StepVrMocapData, sizeof(transform) * STEPBONESNUMS);
#else
	StepVrClient->getData((transform*)GStepMocapData);
#endif
}

void FStepMocapStream::UpdateFrameData_Hand()
{
#if WITH_STEPMAGIC
	FMemory::Memcpy(GStepHandData, GCur_IP_Frame.GRotators, sizeof(V4) * STEPHANDBONESNUMS);
#else
	StepVrClient->GetGloveData(GStepHandData);
#endif
}

void FStepMocapStream::UpdateFrameData_Face()
{
#if WITH_STEPMAGIC
	GStepFaceLength = GCur_IP_Frame.stFaceInf.nValidNum;
	FMemory::Memcpy(GStepFaceData, GCur_IP_Frame.stFaceInf.GFaceData, sizeof(float) * GStepFaceLength);
#else
	StepVrClient->GetFaceData(GStepFaceData, GStepFaceLength);
#endif
}

bool FStepMocapStream::CheckConnectToServer()
{
	bool IsSuccess = false;

#if WITH_STEPMAGIC
	IsSuccess = UpdateStepMagicData();
#else
	if (StepVrClient->IsConnected())
	{
		IsSuccess = true;
	}
	else
	{
		static float GDelayTime = 0.f;
		GDelayTime += GWorld ? UGameplayStatics::GetWorldDeltaSeconds(GWorld) : 0.015f;
		if (GDelayTime > 3)
		{
			GDelayTime = 0.f;
			FString Message = FString::Printf(
				TEXT("StepVrMocap Error %d, ReConnect...%s"), 
				StepVrClient->GetServerStatus(),
				*UsedServerInfo.ServerIP);
			ShowMessage(Message);

			ConnectToServices();
		}
	}
#endif

	return IsSuccess;
}

