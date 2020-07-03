#include "StepVrDataServer.h"
#include "StepIkClientCpp.h"
#include "StepMocapDefine.h"




#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"


#define TESTREPLAY false
#if TESTREPLAY
#include "StepVrStream.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
static FReplayJsonData GReplayJsonData;
static int32			GCurFrames = 0;
#endif


static float		GStepFaceData[STEPFACEMORGHNUMS];
static V4			GStepHandData[STEPHANDBONESNUMS];
static transform	GStepMocapData[STEPBONESNUMS];

void ConvertToUE(transform* InData, TArray<FTransform>& OutData)
{
	OutData.Empty(STEPBONESNUMS);

	FVector TempVec;
	FQuat TempQuat;
	FVector TempScale;

	static const FQuat QBA = FQuat::FQuat(FVector(0, 0, 1), PI);
	static const FQuat QTE = FQuat::FQuat(FVector(0, 0, 1), -PI / 2.0).Inverse()*FQuat::FQuat(FVector(1, 0, 0), -PI / 2.0).Inverse();

	for (int32 i = 0; i < STEPBONESNUMS; i++)
	{
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
			break;
		}

		//根骨骼缩放
		if (i == 0)
		{
			TempScale.X = InData[i].Scale.x;
			TempScale.Y = InData[i].Scale.y;
			TempScale.Z = InData[i].Scale.z;
		}
		else
		{
			TempScale = FVector::OneVector;
		}

		OutData.Add(FTransform(TempQuat, TempVec, TempScale));
	}

	if (OutData.Num() != STEPBONESNUMS)
	{
		OutData.Empty();
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

		TempQuat = TempQuat * QBA;
		TempQuat = QTE * TempQuat;

		if (!TempQuat.IsNormalized())
		{
			FString Message = FString::Printf(TEXT("%f--%f--%f--%f"), TempQuat.X, TempQuat.Y, TempQuat.Z, TempQuat.W);
			StepMocapSpace::ShowMessage(Message);
			OutData.Empty();
			break;
}

		OutData[i] = TempQuat.Rotator();
	}
}

/************************************************************************/
/*						StepMagic  Start                                */
/************************************************************************/
#ifdef WITH_STEPMAGIC

#include "AJA_Corvid44.h"
#include "VirtualShootingDll.h"
#include "StepMagicThirdParty.h"
#include "StepMagicGlobal.h"

//当前帧从StepMagic获取的数据
static MFGValue GCur_IP_Frame;

class FStepMagicData : public StepVrDataServer
{
public:
	~FStepMagicData();

	virtual void Connect2Server(const FString& IP, int32 port) override;
	virtual void DisConnect() override;
	virtual bool IsConnected() override;

	virtual bool HasBodyData() override;
	virtual void GetBodyData(TArray<FTransform>& OutData) override;

	virtual bool HasHandData() override;
	virtual void GetHandData(TArray<FRotator>& OutData) override;

	virtual bool HasFaceData() override;
	virtual void GetFaceData(TMap<FString, float>& OutData) override;

	FDelegateHandle HandleDelegate;
	void StepMagicFrameBegine();

	bool bCurFame = false;
};
FStepMagicData::~FStepMagicData()
{
	
}

void FStepMagicData::Connect2Server(const FString& IP, int32 port)
{
	StepVrDataServer::Connect2Server(IP, port);
	HandleDelegate = GStepMagicFrameBegin.AddRaw(this, &FStepMagicData::StepMagicFrameBegine);
}

void FStepMagicData::DisConnect()
{
	StepVrDataServer::DisConnect();

	GStepMagicFrameBegin.Remove(HandleDelegate);
}

bool FStepMagicData::IsConnected()
{
	return GVirtualShootingDll.IsValid();
}

bool FStepMagicData::HasBodyData()
{
	return bCurFame;
}

void FStepMagicData::GetBodyData(TArray<FTransform>& OutData)
{
	FMemory::Memcpy(GStepMocapData, GCur_IP_Frame.StepVrMocapData, sizeof(transform) * STEPBONESNUMS);
}

bool FStepMagicData::HasHandData()
{
	return bCurFame;
}

void FStepMagicData::GetHandData(TArray<FRotator>& OutData)
{
	FMemory::Memcpy(GStepHandData, GCur_IP_Frame.GRotators, sizeof(V4) * STEPHANDBONESNUMS);
}

bool FStepMagicData::HasFaceData()
{
	return bCurFame;
}

void FStepMagicData::GetFaceData(TMap<FString, float>& OutData)
{
	int32 StepFaceLength = GCur_IP_Frame.stFaceInf.nValidNum;
	FMemory::Memcpy(GStepFaceData, GCur_IP_Frame.stFaceInf.GFaceData, sizeof(float) * StepFaceLength);
}

void FStepMagicData::StepMagicFrameBegine()
{
	bCurFame = false;

	if (!GVirtualShootingDll.IsValid())
	{
		return;
	}

	MultiTracker stMultiTracker;
	GVirtualShootingDll.Pin()->GetMultiTracker(stMultiTracker);

	//更新一帧数据
	MFGKey Keys;
	Keys.strIP = std::string(TCHAR_TO_ANSI(*ServerIP));
	Keys.nPort = ServerPort;
	auto FindValue = stMultiTracker.mapMFG.find(Keys);
	if (FindValue != stMultiTracker.mapMFG.end())
	{
		bCurFame = true;
		GCur_IP_Frame = FindValue->second;
		ReceiveData.ExecuteIfBound();
	}	
}

#endif // WITH_STEPMAGIC

/************************************************************************/
/*						StepMagic  End                                  */
/************************************************************************/


/************************************************************************/
/*						MMAP Start                                      */
/************************************************************************/

//MMAP
class FServicesData : public StepVrDataServer , public StepIK_Client
{
public:
	~FServicesData();

	virtual void Connect2Server(const FString& IP, int32 port) override;
	virtual void DisConnect() override;
	virtual bool IsConnected() override;

	virtual bool HasBodyData() override;
	virtual void GetBodyData(TArray<FTransform>& OutData) override;

	virtual bool HasHandData() override;
	virtual void GetHandData(TArray<FRotator>& OutData) override;

	virtual bool HasFaceData() override;
	virtual void GetFaceData(TMap<FString, float>& OutData) override;

	FDelegateHandle HandleDelegate;
	void MMapFrameBegin();

	virtual void ReplaceSkt(bool IsUse) override
	{
		StepIK_Client::SetSKT(IsUse);
	}


	virtual void RecordStart(const FString& Name) override
	{
		StepIK_Client::StartRecord(TCHAR_TO_ANSI(*Name));
	}


	virtual void RecordStop() override
	{
		StepIK_Client::StopRecord();
	}


	virtual void TPose() override
	{
		StepIK_Client::TPose();
	}

};

FServicesData::~FServicesData()
{
	DisConnect();
}

bool FServicesData::HasBodyData()
{
	return StepIK_Client::HasBodyData();
}

bool FServicesData::HasHandData()
{
	return StepIK_Client::HasGloveData();
}

bool FServicesData::HasFaceData()
{
	return StepIK_Client::HasFaceData();
}

void FServicesData::Connect2Server(const FString& IP, int32 port)
{
	StepVrDataServer::Connect2Server(IP, port);

	StepIK_Client::Connect(TCHAR_TO_ANSI(*IP), port);

	HandleDelegate = FCoreDelegates::OnBeginFrame.AddRaw(this, &FServicesData::MMapFrameBegin);


#if TESTREPLAY
	//数据初始化
	GReplayJsonData.Empty();

	//解析CMD,查找录制路径
	FString RecordPath = TEXT("C:\\Users\\77276\\Desktop\\jump.json");

	bool LoadSuccess = false;
	FString JsonData;
	if (FFileHelper::LoadFileToString(JsonData, *RecordPath))
	{
		auto JsonReader = TJsonReaderFactory<>::Create(JsonData);
		TSharedPtr<FJsonObject> JsonRoot;

		if (FJsonSerializer::Deserialize(JsonReader, JsonRoot))
		{
			if (FJsonObjectConverter::JsonObjectToUStruct<FReplayJsonData>(JsonRoot.ToSharedRef(), &GReplayJsonData))
			{
				LoadSuccess = true;
			}
		}
	}
#endif
}

void FServicesData::DisConnect()
{
	StepVrDataServer::DisConnect();

	FCoreDelegates::OnBeginFrame.Remove(HandleDelegate);

	StepIK_Client::DisConnect();
}

bool FServicesData::IsConnected()
{
#if TESTREPLAY
	GCurFrames++;
	GCurFrames = GCurFrames % GReplayJsonData.EndData.Num();
#endif
	return StepIK_Client::IsConnected();
}

void FServicesData::GetBodyData(TArray<FTransform>& OutData)
{
	//修改根骨骼缩放
#if TESTREPLAY

	//转换动捕数据		
	auto& TempBody = GReplayJsonData.EndData[GCurFrames].body;
	for (int32 i = 0; i < TempBody.Num(); i++)
	{
		GStepMocapData[i].Location.x = TempBody[i].Location.x;
		GStepMocapData[i].Location.y = TempBody[i].Location.y;
		GStepMocapData[i].Location.z = TempBody[i].Location.z;
		GStepMocapData[i].Rotation.w = TempBody[i].Rotation.w;
		GStepMocapData[i].Rotation.x = TempBody[i].Rotation.x;
		GStepMocapData[i].Rotation.y = TempBody[i].Rotation.y;
		GStepMocapData[i].Rotation.z = TempBody[i].Rotation.z;
}
#else
	V3 CurScale;
	StepIK_Client::GetLossyScale(&CurScale);

	StepIK_Client::getData((transform*)GStepMocapData);
	GStepMocapData[0].Scale = CurScale;
#endif

	ConvertToUE(GStepMocapData,OutData);
}


void FServicesData::GetHandData(TArray<FRotator>& OutData)
{
#if TESTREPLAY
	auto& TempHand = GReplayJsonData.EndData[GCurFrames].hand;
	//转换手部数据
	for (int32 i = 0; i < TempHand.Num(); i++)
	{
		GStepHandData[i].w = TempHand[i].w;
		GStepHandData[i].x = TempHand[i].x;
		GStepHandData[i].y = TempHand[i].y;
		GStepHandData[i].z = TempHand[i].z;
	}
#else
	StepIK_Client::GetGloveData(GStepHandData);
#endif

	ConvertToUE(GStepHandData,OutData);
}

void FServicesData::GetFaceData(TMap<FString, float>& OutData)
{
	int32 StepFaceLength = 0;
	StepIK_Client::GetFaceData(GStepFaceData, StepFaceLength);
	
	//static UEnum* GRootEnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("FStepFaceMorghs"), true);
	//GRootEnumPtr->GetNameByValue(i).ToString()
	OutData.Empty(StepFaceLength);
	for (int32 i = 0; i < StepFaceLength; i++)
	{
		if (StepFaceMorphTargets.IsValidIndex(i))
		{
			OutData.Add(StepFaceMorphTargets[i], GStepFaceData[i]);
		}
	}
}


void FServicesData::MMapFrameBegin()
{
	ReceiveData.ExecuteIfBound();
}

/************************************************************************/
/*                          MMAP End                                    */
/************************************************************************/

StepVrDataServer::StepVrDataServer()
{
}


StepVrDataServer::~StepVrDataServer()
{

}

TSharedPtr<StepVrDataServer> StepVrDataServer::CreateServerData()
{
#ifdef WITH_STEPMAGIC
	return MakeShareable(new FStepMagicData());
#else
	return MakeShareable(new FServicesData()); 
#endif // WITH_STEPMAGIC
}

void StepVrDataServer::Connect2Server(const FString& IP, int32 port)
{
	ServerIP = IP;
	ServerPort = port;
}

void StepVrDataServer::DisConnect()
{
	ReceiveData.Unbind();
}


















