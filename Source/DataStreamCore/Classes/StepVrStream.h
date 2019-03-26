// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#pragma once


#include "StepMocapDefine.h"
#include "Animation/AnimNodeBase.h"
#include "LocalFace.h"

class StepIK_Client;
struct FStepVRPNChar;


class FStepMocapStream
{
public:
	FStepMocapStream();
	~FStepMocapStream();

	static GCWT::LocalFace* GetLocalFace();
	static GCWT::LocalGlove* GetLocalGlove();

	//True表示可以释放
	bool ReleaseReference();

	void NeedReference();

	//创建一个链接
	void SetServerInfo(const FMocapServerInfo& ServerInfo);
	const FMocapServerInfo& GetServerInfo();

	void GetBonesTransform(TArray<FTransform>& BonesData);

	bool IsConnected() { return bIsConnected; }

	bool ConnectToServer();
private:
	//当前链接属性
	FMocapServerInfo UsedServerInfo;

	TSharedPtr<StepIK_Client>	StepVrClient;

	int32 ReferenceCount = 0;

	//存储数据
	TArray<FTransform> CacheFrameData;
	uint64	PreCacheFrame;

	bool bIsConnected = false;
};

class STEPVRDATASTREAMCORE_API FStepDataToSkeletonBinding
{
public:

	// Default constructor.
	FStepDataToSkeletonBinding();
	~FStepDataToSkeletonBinding();

	bool ConnectToServer(const FMocapServerInfo& InServerInfo);
	bool IsConnected();

	/**
	 * 全身动捕
	 */
	bool BindToSkeleton(FAnimInstanceProxy* AnimInstanceProxy, TMap<FString, FBoneReference>& BoneReferences);
	bool IsBound();
	bool UpdateFrameData(TArray<FTransform>& outPose);
	int32 GetUE4BoneIndex(int32 SegmentIndex) const;
	float GetFigureScale();

	/**
	 * 手部捕捉
	 */
	bool BindToHandSkeleton(FAnimInstanceProxy* AnimInstanceProxy, TMap<FString, FBoneReference>& BoneReferences);
	bool IsHandBound();
	bool UpdateHandFrameData(TArray<FRotator>& outPose);
	int32 GetUE4HandBoneIndex(int32 SegmentIndex) const;

private:
	FString mSubjectName;
	FString mIpAddress;

	//ServerStream
	TWeakPtr<FStepMocapStream> StepMpcapStream;

	/**
	 * 全身动捕数据
	 */
	TArray<int32> UE4BoneIndices;
	bool mBound;

	/**
	 * 手部动捕数据
	 */
	TArray<int32> UE4HandBoneIndices;
	bool mHandBound;
};