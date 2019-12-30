#pragma once

#include "StepMocapDefine.h"
#include "Animation/AnimNodeBase.h"


class StepVrDataServer;
class FStepMocapStream;


typedef TMap<FString, FBoneReference> BoneMappings;
typedef TMap<uint32, TSharedPtr<FStepMocapStream>> AllStepMocapStreams;

/**
 * 数据流，管理数据来源
 */
class STEPVRDATASTREAMCORE_API FStepMocapStream
{
public:
	static TSharedPtr<FStepMocapStream> GetStepMocapStream(const FMocapServerInfo& ServerInfo);
	static void DestroyStepMocapStream(TSharedPtr<FStepMocapStream> StepMocapStream);

public:
	FStepMocapStream();
	~FStepMocapStream();

	//创建一个链接
	void SetServerInfo(const FMocapServerInfo& ServerInfo);
	const FMocapServerInfo& GetServerInfo();

	TArray<FTransform>&		GetBonesTransform_Body();
	TArray<FRotator>&		GetBonesTransform_Hand();
	TMap<FString, float>&	GetBonesTransform_Face();

	bool IsConnected();
	bool IsBodyConnect();
	bool IsHandConnect();
	bool IsFaceConnect();

protected:
	void EngineBegineFrame();
	void ConnectToServices();
	void DisconnectToServer();

	//True表示可以释放
	bool ReleaseReference();
	void NeedReference();

private:
	//当前链接属性
	FMocapServerInfo UsedServerInfo;
	TSharedPtr<StepVrDataServer> ServerConnect;

	int32 ReferenceCount = 0;

	//动捕存储数据
	TArray<FTransform> CacheBodyFrameData;

	//手套数据
	TArray<FRotator> CacheHandFrameData;

	//面捕数据
	TMap<FString,float> CacheFaceFrameData;

	FDelegateHandle	EngineBeginHandle;

	//所有连接
	static AllStepMocapStreams AllStreams;
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

	struct FMapBone
	{
		uint8			StepBoneIndex;
		int32			UeBoneIndex;
		EMapBoneType	MapBoneType;
		FTransform		BoneData;
		FMapBone()
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

	struct FMorphData
	{
		FName StepMarphName;
		FName UE4MarphName;
		float MorphValue;
	};

public:
	// Default constructor.
	FStepDataToSkeletonBinding();
	~FStepDataToSkeletonBinding();

	bool ConnectToServer(const FMocapServerInfo& InServerInfo);

	/**
	 * 全身动捕
	 */
	void BindToSkeleton(FAnimInstanceProxy* AnimInstanceProxy, BoneMappings& BodyBoneReferences, BoneMappings& HandBoneReferences);
	void UpdateSkeletonFrameData();
	const TArray<int32>& GetUE4NeedUpdateBones();
	const FStepDataToSkeletonBinding::FMapBone& GetUE4BoneIndex(int32 SegmentIndex) const;
	const TArray<FStepDataToSkeletonBinding::FMapBone>& GetUE4Bones();

	/**
	 * 面部捕捉
	 */
	void BindToFaceMorghTarget(FAnimInstanceProxy* AnimInstanceProxy, TMap<FString, FString>& MorghTarget);
	void UpdateFaceFrameData();
	bool IsFaceBound();
	const TArray<FStepDataToSkeletonBinding::FMorphData>& GetUE4FaceData();

	/**
	 * 骨骼缩放
	 */
	const FVector& GetSkeletonScale();

	void ResetMocapStream();

private:
	FMocapServerInfo CacheServerInfo;

	
	//ServerStream
	TSharedPtr<FStepMocapStream> StepMpcapStream;

	/**
	 * 当前骨骼缩放
	 */
	FVector SkeletonScale;

	/**
	 * 全身动捕数据
	 */
	TArray<FMapBone> UE4BoneIndices;
	TArray<int32> UE4NeedUpdateBones;
	/**
	 * 面部捕捉
	 */
	TArray<FStepDataToSkeletonBinding::FMorphData> UE4FaceData;
	bool mFaceBound = true;

	//联网
	uint64 CurFrame = 0;
	double DetalTime = 0.f;
};