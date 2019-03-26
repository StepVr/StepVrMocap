// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#pragma once

#include "CoreMinimal.h"


struct FMocapServerInfo 
{
	FString ServerIP;
	int32	ServerPort;

	FMocapServerInfo()
	{
		ServerIP = "";
		ServerPort = 0;
	};

	bool IsEmpty() const
	{
		return ServerPort == 0 || ServerIP.IsEmpty();
	}

	bool IsEqual(const FMocapServerInfo& InMocapServerInfo) const
	{
		return ServerPort == InMocapServerInfo.ServerPort && ServerIP.Equals(InMocapServerInfo.ServerIP);
	}
};

/************************************************************************/
//骨架数
#define  STEPBONESNUMS 22
#define  STEPHANDBONESNUMS 32


//骨架对应父级
const TArray<uint8> StepBonesID = { 0, 0, 1, 2, 3, 4, 3, 6, 7, 8, 3, 10, 11, 12, 1, 14, 15, 16, 1, 18, 19, 20 };

//原始骨架名
const TArray<FString> StepBoneNames = {
	/* UE骨架      --    服务骨架    */
	"root",				//"root"
	"Hips" ,			//"pelvis"
	"Spine1",			//"spine"
	"Spine3",			//"chest"
	"neck",				//"neck"
	"head",				//"head"
	"leftShoulder",		//"LeftShoulder"
	"LeftArm",			//"leftUpperArm"
	"leftForearm",		//"leftForearm"
	"leftHand",			//"leftHand"
	"rightShoulder" ,	//"RightShoulder"
	"RightArm",			//"rightUpperArm"
	"rightForearm",		//"RightForeArm"
	"rightHand",		//"rightHand"
	"LeftUpLeg",		//"leftThigh"
	"LeftLeg",			//"leftCalf",
	"leftFoot",			//"LeftFoot"
	"LeftToeBase",		//"leftToes",
	"RightUpLeg",		//"rightThigh",
	"RightLeg",			//"rightCalf",
	"rightFoot",		//"RightFoot"
	"RightToeBase",		//"rightToes"
};


//原始手部骨架名
const TArray<FString> StepHandBoneNames = {
	//左手
	"LeftFingersBase",
	"LeftHandThumb1" ,	
	"LeftHandThumb2",	
	"LeftHandThumb3",	
	"LeftHandIndex1",	
	"LeftHandIndex2",	
	"LeftHandIndex3",
	"LeftHandMiddle1",
	"LeftHandMiddle2",
	"LeftHandMiddle3",	
	"LeftHandRing1" ,
	"LeftHandRing2",	
	"LeftHandRing3",	
	"LeftHandPinky1",	
	"LeftHandPinky2",	
	"LeftHandPinky3",
	//右手
	"RightFingersBase" ,
	"RightThumb1",
	"RightThumb2",
	"RightThumb3",
	"RightIndex1",
	"RightIndex2",
	"RightIndex3",
	"RightMiddle1",
	"RightMiddle2",
	"RightMiddle3" ,
	"RightRing1",
	"RightRing2",
	"RightRing3",
	"RightPinky1",
	"RightPinky2",
	"RightPinky3",
};
/************************************************************************/