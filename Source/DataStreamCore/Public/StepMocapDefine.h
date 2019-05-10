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
const TArray<uint8> StepHandBonesID = { 100,0,1,2,0,4,5,0,7,8,0,10,11,0,13,14,101,16,17,18,16,20,21,16,23,24,16,26,27,16,29,30 };

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
	"LeftFingersBase",				//0
	"LeftHandThumb1" ,				//1
	"LeftHandThumb2",				//2
	"LeftHandThumb3",				//3
	"LeftHandIndex1",				//4
	"LeftHandIndex2",				//5
	"LeftHandIndex3",				//6
	"LeftHandMiddle1",				//7
	"LeftHandMiddle2",				//8
	"LeftHandMiddle3",				//9
	"LeftHandRing1" ,				//10
	"LeftHandRing2",				//11
	"LeftHandRing3",				//12
	"LeftHandPinky1",				//13
	"LeftHandPinky2",				//14
	"LeftHandPinky3",				//15

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