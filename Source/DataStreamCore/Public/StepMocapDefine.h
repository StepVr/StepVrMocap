// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Stats.h"

DECLARE_LOG_CATEGORY_EXTERN(LogStepMocap, Log, All)

//Stat stepmocap
DECLARE_STATS_GROUP(TEXT("stepmocap"), STATGROUP_STEPMOCAP, STATCAT_Advanced);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Initialize_AnyThread"), COUNT_Initialize_AnyThread, STATGROUP_STEPMOCAP);
DECLARE_CYCLE_STAT(TEXT("Initialize_AnyThread"), STAT_Initialize_AnyThread, STATGROUP_STEPMOCAP);
DECLARE_CYCLE_STAT(TEXT("Update_AnyThread"), STAT_Update_AnyThread, STATGROUP_STEPMOCAP);
DECLARE_CYCLE_STAT(TEXT("EvaluateComponentSpace_AnyThread"), STAT_EvaluateComponentSpace_AnyThread, STATGROUP_STEPMOCAP);

/**
 * 全局函数
 */
void ShowMessage(const FString& Log);
FString GetLocalIP();
FString Convert2LocalIP(const FString& NewIP);


/**
 * 联机状态
 */
enum FStepControllState
{
	Invalid,
	Local,
	Remote,
};


/**
 * 连接服务信息
 */
struct FMocapServerInfo 
{
	FString ServerIP;
	int32	ServerPort;

	bool EnableHand;
	bool EnableFace;

	//同步
	bool IsLocal;
	uint32 AddrValue;

	FMocapServerInfo():
		ServerIP(""),
		ServerPort(0),
		EnableHand(false),
		EnableFace(false),
		IsLocal(true),
		AddrValue(0)
	{
	};

	bool IsEmpty() const
	{
		return ServerPort == 0 || ServerIP.IsEmpty() || ServerIP.Equals("None");
	}

	bool IsEqual(const FMocapServerInfo& InMocapServerInfo) const
	{
		return ServerPort == InMocapServerInfo.ServerPort && ServerIP.Equals(InMocapServerInfo.ServerIP);
	}
	bool IsEqual(const FString& NewServerIP, int32 NewPort) const
	{
		return ServerIP.Equals(NewServerIP) && ServerPort == NewPort;
	}
};


/************************************************************************/
/*身体捕捉
/************************************************************************/
#define  STEPBONESNUMS 22

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


/************************************************************************/
/*手部捕捉
/************************************************************************/
#define  STEPHANDBONESNUMS 32

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
	"RightHandFingersBase" ,		//16
	"RightHandThumb1",
	"RightHandThumb2",
	"RightHandThumb3",
	"RightHandIndex1",
	"RightHandIndex2",
	"RightHandIndex3",
	"RightHandMiddle1",
	"RightHandMiddle2",
	"RightHandMiddle3" ,
	"RightHandRing1",
	"RightHandRing2",
	"RightHandRing3",
	"RightHandPinky1",
	"RightHandPinky2",
	"RightHandPinky3",
};



/************************************************************************/
/* 面部捕捉  
/* 面部点配对，需要在插件启动时读取 C://StepFace//config//configBlends.json   */
/************************************************************************/
#define  STEPFACEMORGHNUMS 130
void LoadMorphTargets();
extern TArray<FString> StepFaceMorphTargets;