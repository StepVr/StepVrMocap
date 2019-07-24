// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "StepMocapDefine.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogStepMocap, Log, All)

/************************************************************************/
void ShowMessage(const FString& Log);
FString GetLocalIP();
FString Convert2LocalIP(const FString& NewIP);

void LoadMorphTargets();
/************************************************************************/


enum FStepControllState
{
	Invalid,
	Local,
	Remote,
};



struct FMocapServerInfo 
{
	FString ServerIP;
	int32	ServerPort;

	bool EnableHand;
	bool EnableFace;

	FMocapServerInfo():
		ServerIP(""),
		ServerPort(0),
		EnableHand(false),
		EnableFace(false)
	{
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
/*身体捕捉
/************************************************************************/
#define  STEPBONESNUMS 22

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
/************************************************************************/
/*身体捕捉
/************************************************************************/


/************************************************************************/
/*手部捕捉
/************************************************************************/
#define  STEPHANDBONESNUMS 32

const TArray<uint8> StepHandBonesID = { 100,0,1,2,0,4,5,0,7,8,0,10,11,0,13,14,101,16,17,18,16,20,21,16,23,24,16,26,27,16,29,30 };

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
/*手部捕捉
/************************************************************************/



/************************************************************************/
/* 面部捕捉                                                              */
/************************************************************************/
#define  STEPFACEMORGHNUMS 130

UENUM(BlueprintType)
enum class FStepFaceMorghs : uint8
{
	Expressions_browsMidVert_min = 0,
	Expressions_browsMidVert_max = 1,
	Expressions_browOutVertL_min = 2,
	Expressions_browOutVertL_max = 3,
	Expressions_browOutVertR_min = 4,
	Expressions_browOutVertR_max = 5,
	Expressions_browSqueezeL_min = 6,
	Expressions_browSqueezeL_max = 7,
	Expressions_browSqueezeR_min = 8,
	Expressions_browSqueezeR_max = 9,

	Expressions_eyeClosedL_min = 10,
	Expressions_eyeClosedL_max = 11,
	Expressions_eyeClosedR_min = 12,
	Expressions_eyeClosedR_max = 13,
	Expressions_eyeClosedPressureL_min = 14,
	Expressions_eyeClosedPressureL_max = 15,
	Expressions_eyeClosedPressureR_min = 16,
	Expressions_eyeClosedPressureR_max = 17,
	Expressions_eyeSquintL_min = 18,
	Expressions_eyeSquintL_max = 19,

	Expressions_eyeSquintR_min = 20,
	Expressions_eyeSquintR_max = 21,
	Expressions_cheekSneerL_max = 22,
	Expressions_cheekSneerR_max = 23,
	Expressions_mouthSmile_min = 24,
	Expressions_mouthSmile_max = 25,
	Expressions_mouthOpenAggr_min = 26,
	Expressions_mouthOpenAggr_max = 27,
	Expressions_nostrilsExpansion_min = 28,
	Expressions_nostrilsExpansion_max = 29,

	Expressions_mouthSmileL_max = 30,
	Expressions_mouthSmileR_max = 31,
	Expressions_mouthSmileOpen_min = 32,
	Expressions_mouthSmileOpen_max = 33,
	Expressions_mouthSmileOpen2_min = 34,
	Expressions_mouthSmileOpen2_max = 35,
	Expressions_mouthLowerOut_min = 36,
	Expressions_mouthLowerOut_max = 37,
	Expressions_mouthClosed_min = 38,
	Expressions_mouthClosed_max = 39,

	Expressions_mouthOpen_min = 40,
	Expressions_mouthOpen_max = 41,
	Expressions_mouthOpenTeethClosed_min = 42,
	Expressions_mouthOpenTeethClosed_max = 43,
	Expressions_mouthOpenLarge_min = 44,
	Expressions_mouthOpenLarge_max = 45,
	Expressions_mouthChew_min = 46,
	Expressions_mouthChew_max = 47,
	Expressions_eyesHoriz_min = 48,
	Expressions_eyesHoriz_max = 49,
	Expressions_eyesVert_min = 50,
	Expressions_eyesVert_max = 51
};
extern TArray<FString> StepFaceMorphTargets;

//USTRUCT(BlueprintType)
//struct FStepVrFaceAtt
//{
//	GENERATED_USTRUCT_BODY()
//
//public:
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_abdomExpansion_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_abdomExpansion_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browOutVertL_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browOutVertL_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browOutVertR_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browOutVertR_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browSqueezeL_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browSqueezeL_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browSqueezeR_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browSqueezeR_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browsMidVert_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_browsMidVert_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_cheekSneerL_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_cheekSneerR_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_chestExpansion_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_chestExpansion_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_deglutition_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_deglutition_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeClosedL_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeClosedL_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeClosedPressureL_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeClosedPressureL_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeClosedPressureR_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeClosedPressureR_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeClosedR_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeClosedR_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeSquintL_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeSquintL_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeSquintR_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyeSquintR_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyesHoriz_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyesHoriz_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyesVert_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_eyesVert_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_jawHoriz_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_jawHoriz_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_jawOut_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_jawOut_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthBite_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthBite_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthChew_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthChew_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthClosed_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthClosed_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthHoriz_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthHoriz_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthInflated_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthInflated_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthLowerOut_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthLowerOut_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpenAggr_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpenAggr_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpenHalf_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpenLarge_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpenLarge_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpenO_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpenO_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpenTeethClosed_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpenTeethClosed_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpen_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthOpen_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthSmileL_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthSmileOpen2_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthSmileOpen2_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthSmileOpen_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthSmileOpen_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthSmileR_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthSmile_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_mouthSmile_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_nostrilsExpansion_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_nostrilsExpansion_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_pupilsDilatation_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_pupilsDilatation_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_tongueHoriz_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_tongueHoriz_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_tongueOutPressure_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_tongueOut_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_tongueOut_min;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_tongueTipUp_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_tongueVert_max;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//		float Expressions_tongueVert_min;
//
//};
/************************************************************************/
/*        面部捕捉                                                       */
/************************************************************************/