// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#pragma once


#include "StepMocapDefine.h"
#include "Animation/AnimNodeBase.h"


class StepIK_Client;
struct FStepVRPNChar;

typedef TMap<FString, FBoneReference> BoneMappings;


class FStepMocapStream
{
public:
	FStepMocapStream();
	~FStepMocapStream();

	//True表示可以释放
	bool ReleaseReference();

	void NeedReference();

	//创建一个链接
	void SetServerInfo(const FMocapServerInfo& ServerInfo);
	const FMocapServerInfo& GetServerInfo();

	TArray<FTransform>&	GetBonesTransform_Body();
	TArray<FRotator>&	GetBonesTransform_Hand();
	void GetBonesTransform_Face(TMap<FString,float>& BonesData);

	bool IsConnected();
	bool IsBodyConnect();
	bool IsHandConnect();
	bool IsFaceConnect();

protected:
	void EngineBegineFrame();
#if WITH_STEPMAGIC
	bool UpdateStepMagicData();
#endif
	void UpdateFrameData_Body();
	void UpdateFrameData_Hand();
	void UpdateFrameData_Face();

	bool CheckConnectToServer();
	void ConnectToServices();
	void ConnectToStepMagic();
	void DisconnectToServer();

private:
	//当前链接属性
	FMocapServerInfo UsedServerInfo;

	TSharedPtr<StepIK_Client>	StepVrClient;

	int32 ReferenceCount = 0;

	//动捕存储数据
	TArray<FTransform> CacheBodyFrameData;

	//手套数据
	TArray<FRotator> CacheHandFrameData;

	//面捕数据
	TMap<FString,float> CacheFaceFrameData;

	FDelegateHandle	EngineBeginHandle;
};

class STEPVRDATASTREAMCORE_API FStepDataToSkeletonBinding
{
public:
	enum class EMapBoneType : uint8
	{
		Bone_Body,
		Bone_Hand,
		InValid,
	};

	struct MapBone
	{
		uint8			StepBoneIndex;
		int32			UeBoneIndex;
		EMapBoneType	MapBoneType;
		FTransform		BoneData;
		MapBone()
		{
			Reset();
		}

		void Reset()
		{
			StepBoneIndex = 0;
			UeBoneIndex = INDEX_NONE;
			MapBoneType = EMapBoneType::InValid;
		}
	};

	// Default constructor.
	FStepDataToSkeletonBinding();
	~FStepDataToSkeletonBinding();

	bool ConnectToServer(const FMocapServerInfo& InServerInfo);
	bool IsConnected();

	/**
	 * 全身动捕
	 */
	void BindToSkeleton(FAnimInstanceProxy* AnimInstanceProxy, BoneMappings& BodyBoneReferences, BoneMappings& HandBoneReferences);
	bool IsBodyBound();
	void UpdateSkeletonFrameData();
	const FStepDataToSkeletonBinding::MapBone& GetUE4BoneIndex(int32 SegmentIndex) const;
	const TArray<FStepDataToSkeletonBinding::MapBone>& GetUE4Bones();
	float GetFigureScale();

	/**
	 * 手部捕捉
	 */
	bool BindToHandSkeleton(FAnimInstanceProxy* AnimInstanceProxy, TMap<FString, FBoneReference>& BoneReferences);
	bool IsHandBound();
	int32 GetUE4HandBoneIndex(int32 SegmentIndex) const;

	/**
	 * 面部捕捉
	 */
	bool BindToFaceMorghTarget(FAnimInstanceProxy* AnimInstanceProxy,TArray<FString>& MorghTarget);
	bool UpdateFaceFrameData(TMap<FString,float>& outPose);
	bool IsFaceBound();

private:
	FMocapServerInfo CacheServerInfo;;

	//ServerStream
	TSharedPtr<FStepMocapStream> StepMpcapStream;

	/**
	 * 全身动捕数据
	 */
	TArray<MapBone> UE4BoneIndices;
	bool mBound = false;

	/**
	 * 手部动捕数据
	 */
	TArray<int32> UE4HandBoneIndices;
	bool mHandBound = false;

	/**
	 * 面部捕捉
	 */
	bool mFaceBound = true;
};