#include "StepVrStream.h"
#include "StepVrDataServer.h"
#include "StepVrSkt.h"
#include "StepVrDataServer.h"
#include "StepIkClientCpp.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/MorphTarget.h"
#include "Misc/CoreDelegates.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"



namespace StepMocapServers
{
	static bool IsConnected = false;
//	static TMap<uint32, TSharedPtr<FStepMocapStream>> AllStreams;

//	TSharedRef<FStepMocapStream> GetStream(const FMocapServerInfo& InServerInfo);
//	void ReturnStream(TSharedRef<FStepMocapStream> StepMocapStream);

//	uint32 GetServerID(const FMocapServerInfo& InServerInfo);
}









// Default constructor.
FStepDataToSkeletonBinding::FStepDataToSkeletonBinding()
{
}

FStepDataToSkeletonBinding::~FStepDataToSkeletonBinding()
{
	ResetMocapStream();
}


bool FStepDataToSkeletonBinding::ConnectToServer(const FMocapServerInfo& InServerInfo)
{
	bool Success = false;

	if (InServerInfo.IsEmpty())
	{
		return Success;
	}

	CacheServerInfo = InServerInfo;
	ResetMocapStream();


	switch (CacheServerInfo.StepControllState)
	{
	case Local_Replicate_N:
	case Local_Replicate_Y:
	{
		//创建本地Stream
		StepMpcapStream = FStepMocapStream::GetStepMocapStream(CacheServerInfo);
		Success = StepMpcapStream.IsValid();
	}
	break;
	case Remote_Replicate_Y:
	{
		Success = true;
	}break;
	}

	return Success;
}

void FStepDataToSkeletonBinding::BindToSkeleton(FAnimInstanceProxy* AnimInstanceProxy, BoneMappings& BodyBoneReferences, BoneMappings& HandBoneReferences)
{
	const FBoneContainer& BoneContainer = AnimInstanceProxy->GetRequiredBones();
	auto RefSkeleton = BoneContainer.GetReferenceSkeleton();

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

		Ref->Initialize(BoneContainer);

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

		Ref->Initialize(BoneContainer);

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


	for (int Index = 0; Index < UE4BoneIndices.Num(); Index++)
	{
		//当前节点
		int32 CurIdnex = UE4BoneIndices[Index].UeBoneIndex;
		UE4NeedUpdateBones.AddUnique(CurIdnex);
	
		FName BoneName = RefSkeleton.GetBoneName(CurIdnex);
		//UE_LOG(LogTemp, Log, TEXT("CurName %d - %s"), CurIdnex, *BoneName.ToString());

		//父节点
		
		while ((CurIdnex = BoneContainer.GetParentBoneIndex(CurIdnex)) > 0)
		{
			if (UE4NeedUpdateBones.Find(CurIdnex) == INDEX_NONE)
			{
				FName BoneName1 = RefSkeleton.GetBoneName(CurIdnex);
				//UE_LOG(LogTemp,Log,TEXT("   ParentName %d - %s"),CurIdnex,*BoneName1.ToString());
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

void FStepDataToSkeletonBinding::ResetMocapStream()
{
	if (StepMpcapStream.IsValid())
	{
		FStepMocapStream::DestroyStepMocapStream(StepMpcapStream.ToSharedRef());
		StepMpcapStream.Reset();
	}
}

void FStepDataToSkeletonBinding::LoadReplayData(FString ReplayFile)
{
	//数据初始化
	ReplayJsonData.Empty();
	iCurUseDataFrames = 0;
	iCurFrames = 0;

	//解析CMD,查找录制路径
	FString RecordPath;
	if (ReplayFile.IsEmpty())
	{
		FParse::Value(FCommandLine::Get(), TEXT("-StepRecordPath="), RecordPath);
		RecordPath.Append(TEXT("\\RecordData.json"));
	}
	else
	{
		RecordPath = ReplayFile;
	}

	bool LoadSuccess = false;
	FString JsonData;
	if (FFileHelper::LoadFileToString(JsonData, *RecordPath))
	{
		auto JsonReader = TJsonReaderFactory<>::Create(JsonData);
		TSharedPtr<FJsonObject> JsonRoot;

		if (FJsonSerializer::Deserialize(JsonReader, JsonRoot))
		{
			if (FJsonObjectConverter::JsonObjectToUStruct<FReplayJsonData>(JsonRoot.ToSharedRef(), &ReplayJsonData))
			{
				LoadSuccess = true;
			}
		}
	}

	if (LoadSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Step Load ReplayJson Success"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Step Load ReplayJson Faild"));
	}
}

void FStepDataToSkeletonBinding::UpdateSkeletonFrameData()
{
	TArray<FTransform> BodyData;
	TArray<FRotator> HandData;
	BonesTransform(BodyData, HandData);

	bool bBodyData = BodyData.Num() == STEPBONESNUMS;
	bool bHandData = HandData.Num() == STEPHANDBONESNUMS;

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

void FStepDataToSkeletonBinding::BonesTransform(TArray<FTransform>& Bones, TArray<FRotator>&Hands)
{
	if (StepMpcapStream.IsValid())
	{
		Bones = StepMpcapStream->GetBonesTransform_Body();
		Hands = StepMpcapStream->GetBonesTransform_Hand();
		return;
	}

		//找到对时后动捕数据
	float Interval = 1.f / 30 * 1000;

	float TargetTime = iCurFrames * Interval;

	float curCompare = 10000.f;
	for (int32 i = iCurUseDataFrames; i < ReplayJsonData.EndData.Num(); i++)
	{
		float curCompareInterval = FMath::Abs(ReplayJsonData.EndData[i].timeStamp - TargetTime);
		if (curCompare > curCompareInterval)
		{
			curCompare = curCompareInterval;
			iCurUseDataFrames = i;
		}
		else
		{
			break;
		}
	}

	//没有找到对应数据，退出
	if (curCompare > 1000)
	{
		UE_LOG(LogTemp, Log, TEXT("Step Can Not Find Replay Data"));
		return;
	}

	iCurFrames++;


	//进行数据转换为默认数据，配合通用算法，防止转换出错
	{
		transform	StepBodyData[STEPBONESNUMS];
		V4			StepHandData[STEPHANDBONESNUMS];
		auto& TempBody = ReplayJsonData.EndData[iCurUseDataFrames].body;
		auto& TempHand = ReplayJsonData.EndData[iCurUseDataFrames].hand;

		//转换动捕数据
		for (int32 i = 0; i < TempBody.Num(); i++)
		{
			StepBodyData[i].Location.x = TempBody[i].Location.x;
			StepBodyData[i].Location.y = TempBody[i].Location.y;
			StepBodyData[i].Location.z = TempBody[i].Location.z;
			StepBodyData[i].Rotation.w = TempBody[i].Rotation.w;
			StepBodyData[i].Rotation.x = TempBody[i].Rotation.x;
			StepBodyData[i].Rotation.y = TempBody[i].Rotation.y;
			StepBodyData[i].Rotation.z = TempBody[i].Rotation.z;
		}
		ConvertToUE(StepBodyData, Bones);

		//转换手部数据
		for (int32 i = 0; i < TempHand.Num(); i++)
		{
			StepHandData[i].w = TempHand[i].w;
			StepHandData[i].x = TempHand[i].x;
			StepHandData[i].y = TempHand[i].y;
			StepHandData[i].z = TempHand[i].z;
		}
		ConvertToUE(StepHandData, Hands);
	}
}


const TArray<int32>& FStepDataToSkeletonBinding::GetUE4NeedUpdateBones()
{
	return UE4NeedUpdateBones;
}



/**
 * FStepMocapStream
 */
AllStepMocapStreams FStepMocapStream::AllStreams = {};

uint32 GetServerID(const FMocapServerInfo& InServerInfo)
{
	FString ServerString = InServerInfo.ServerIP + FString::FormatAsNumber(InServerInfo.ServerPort);
	uint32 ServerID = GetTypeHash(ServerString);
	return ServerID;
}

FStepMocapStream::FStepMocapStream()
{

}

FStepMocapStream::~FStepMocapStream()
{
	DisconnectToServer();
}

TSharedPtr<FStepMocapStream> FStepMocapStream::GetActorMocapStream(uint32 UID)
{
	for (auto& Pair : FStepMocapStream::AllStreams)
	{
		if (Pair.Value->HasActorWithUID(UID))
		{
			return Pair.Value;
		}
	}

	return nullptr;
}

TSharedPtr<FStepMocapStream> FStepMocapStream::GetStepMocapStream(FMocapServerInfo& ServerInfo, bool AlwaysCreat)
{
	uint32 ServerID = GetServerID(ServerInfo);

	TSharedPtr<FStepMocapStream> NewStream;

	auto Temp = FStepMocapStream::AllStreams.Find(ServerID);
	if (Temp)
	{
		NewStream = *Temp;
	}
	else if(AlwaysCreat)
	{
		NewStream = MakeShareable(new FStepMocapStream());
		NewStream->SetServerInfo(ServerInfo);
		FStepMocapStream::AllStreams.Add(ServerID, NewStream);
	}

	if (NewStream.IsValid())
	{
		NewStream->AddActorOwner(ServerInfo);
		NewStream->NeedReference();
	}
	else
	{
		FString Message = FString::Printf(TEXT("StepMocapGetStreamFaild : ServerIP:%s , ServerPort:%d"), *ServerInfo.ServerIP, ServerInfo.ServerPort);
		StepMocapSpace::ShowMessage(Message);
	}

	return NewStream;
}

void FStepMocapStream::DestroyStepMocapStream(TSharedPtr<FStepMocapStream> StepMocapStream)
{
	if (!StepMocapStream.IsValid())
	{
		return;
	}

	uint32 ServerID = GetServerID(StepMocapStream->GetServerInfo());
	auto Temp = AllStreams.Find(ServerID);

	bool NeedRelease = (Temp != nullptr) && (*Temp)->ReleaseReference();
	if (NeedRelease)
	{
		AllStreams.Remove(ServerID);
	}
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

void FStepMocapStream::ReplcaeSkt(const FString& NewSktName)
{
	do 
	{
		if (!STEPVRSKT->ReplcaeSkt(NewSktName))
		{
			break;
		}

		if (!ServerConnect.IsValid())
		{
			break;
		}

		ServerConnect->ReplaceSkt(!NewSktName.IsEmpty());
	} while (0);
}

void FStepMocapStream::RecordStart(const FString& Name)
{
	if(ServerConnect.IsValid())
	{
		ServerConnect->RecordStart(Name);
	}
}

void FStepMocapStream::RecordStop()
{
	if (ServerConnect.IsValid())
	{
		ServerConnect->RecordStop();
	}
}

void FStepMocapStream::TPose()
{
	if (ServerConnect.IsValid())
	{
		ServerConnect->TPose();
	}
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
	return ServerConnect.IsValid() && ServerConnect->IsConnected();
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

void FStepMocapStream::AddActorOwner(FMocapServerInfo& ServerInfo)
{
	if (CacheActors.Find(ServerInfo.UID) == nullptr)
	{
		CacheActors.Add(ServerInfo.UID,ServerInfo);
	}
}


void FStepMocapStream::RemoveActorOwner(FString& ActorName)
{
	//auto Pair = CacheActors.Find(ActorName);
	//if (Pair)
	//{
	//	CacheActors.Remove(Pair->OwnerActorName);
	//}
}

FMocapServerInfo* FStepMocapStream::HasActorWithUID(uint32 UID)
{
	return  CacheActors.Find(UID);
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
	
	StepMocapSpace::ShowMessage(Message);
}
void FStepMocapStream::DisconnectToServer()
{
	ServerConnect = nullptr;

	FString Message = FString::Printf(TEXT("StepMocapStream Delete : %s"), *UsedServerInfo.ServerIP);
	
	StepMocapSpace::ShowMessage(Message);
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
		//if (UsedServerInfo.StepControllState == FStepControllState::Local_Replicate_Y && 
		//	STEPVR_SERVER_IsValid)
		//{
		//	STEPVR_SERVER->StepMocapSendData(CacheBodyFrameData);
		//}
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