// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#pragma once


#include "StepMocapDefine.h"
#include "Animation/AnimNodeBase.h"

class StepIK_Client;
struct FStepVRPNChar;


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

	void GetBonesTransform_Body(TArray<FTransform>& BonesData);
	void GetBonesTransform_Hand(TArray<FRotator>& BonesData);
	void GetBonesTransform_Face(TMap<FString,float>& BonesData);

	bool IsConnected()		{ return bClientConnected; }
	bool IsConnectedBody()	{ return bBodyConnected; }
	bool IsConnectedHand()	{ return bHandConnected; }
	bool IsConnectedFace()	{ return bFaceConnected; }

protected:
	void EngineBegineFrame();

	void ConnectToServer();
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

	bool bClientConnected = false;
	bool bBodyConnected = false;
	bool bHandConnected = false;
	bool bFaceConnected = false;
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
	bool IsBodyBound();
	bool UpdateBodyFrameData(TArray<FTransform>& outPose);
	int32 GetUE4BoneIndex(int32 SegmentIndex) const;
	float GetFigureScale();

	/**
	 * 手部捕捉
	 */
	bool BindToHandSkeleton(FAnimInstanceProxy* AnimInstanceProxy, TMap<FString, FBoneReference>& BoneReferences);
	bool IsHandBound();
	bool UpdateHandFrameData(TArray<FRotator>& outPose);
	int32 GetUE4HandBoneIndex(int32 SegmentIndex) const;

	/**
	 * 面部捕捉
	 */
	bool BindToFaceMorghTarget(FAnimInstanceProxy* AnimInstanceProxy,TArray<FString>& MorghTarget);
	bool UpdateFaceFrameData(TMap<FString,float>& outPose);
	bool IsFaceBound();

private:
	FString mSubjectName;
	FString mIpAddress;

	//ServerStream
	TSharedPtr<FStepMocapStream> StepMpcapStream;

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

	/**
	 * 面部捕捉
	 */
	bool mFaceBound;
};