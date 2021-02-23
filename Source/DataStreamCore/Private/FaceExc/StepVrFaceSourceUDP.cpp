#include "FaceExc/StepVrFaceSourceUDP.h"
#include "FaceExc/StepVrFaceSourceStruct.h"

#include "Sockets.h"
#include "SocketSubsystem.h"

#include "Math/UnrealMathUtility.h"

#include "LiveLinkTypes.h"
#include "JsonObjectConverter.h"



static FName ParseEnumName1(FName EnumName)
{
	const int32 BlendShapeEnumNameLength = 19;
	FString EnumString = EnumName.ToString();
	return FName(*EnumString.Right(EnumString.Len() - BlendShapeEnumNameLength));
}

FStepListenerToSelf* FStepListenerToSelf::CreateRemoteListener()
{
	static FStepListenerToSelf* Listener = nullptr;
	if (Listener == nullptr)
	{
		Listener = new FStepListenerToSelf();
		if (!Listener->InitReceiveSocket())
		{
			delete Listener;
			Listener = nullptr;
		}
	}

	return Listener;
}

FStepListenerToSelf::FStepListenerToSelf()
{

}

FStepListenerToSelf::~FStepListenerToSelf()
{
	if (RecvSocket != nullptr)
	{
		RecvSocket->Close();

		if (!IsEngineExitRequested())
		{
			ISocketSubsystem* SocketSub = ISocketSubsystem::Get();
			SocketSub->DestroySocket(RecvSocket);
		}
	}
}

void FStepListenerToSelf::Tick(float DeltaTime)
{
	uint32 Size;
	while (RecvSocket->HasPendingData(Size))
	{
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
		TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
		
		int32 Read = 0;
		RecvBuffer.Init(0, FMath::Min(Size, (uint32)1024));
		if (RecvSocket->RecvFrom(RecvBuffer.GetData(), RecvBuffer.Num(), Read, *Sender))
		{
			FStepFaceData StepFaceData;
			//FString command = TEXT("{\"data\": {\"jawOpen\": 0.0,\"jawOpe1\" : 0.0},\"timestamp\" : 0.0}");			
			
			FString command((char*)RecvBuffer.GetData());
			FJsonObjectConverter::JsonObjectStringToUStruct<FStepFaceData>(command, &StepFaceData, 0, 0);

			FName SubjectName = FName(*StepFaceData.socket_id);

			//缓存StaticData
			FLiveLinkBaseStaticData& BaseStaticData = BaseStaticDatas.FindOrAdd(SubjectName);
			FLiveLinkBaseFrameData& BaseFrameData = BaseFrameDatas.FindOrAdd(SubjectName);

			float TempScale = 1.f;
			if (auto ScalePtr = BaseScale.Find(SubjectName))
			{
				TempScale = *ScalePtr;
			}

			BaseStaticData.PropertyNames.Reset(StepFaceData.data.Num());
			BaseFrameData.PropertyValues.Reset(StepFaceData.data.Num());
			
			for (auto TempPair : StepFaceData.data)
			{
				BaseFrameData.PropertyValues.Add(FMath::Clamp(TempPair.Value * TempScale, 0.f, 1.f));
				BaseStaticData.PropertyNames.Add(FName(*TempPair.Key));
			}
		}
	}
}

bool FStepListenerToSelf::InitReceiveSocket()
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
	TSharedRef<FInternetAddr> Addr = SocketSubsystem->GetLocalBindAddr(*GLog);

	Addr->SetPort(11113);

	RecvSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("FaceStepUDP socket"), Addr->GetProtocolType());
	if (RecvSocket != nullptr)
	{
		RecvSocket->SetReuseAddr();
		RecvSocket->SetNonBlocking();
		RecvSocket->SetRecvErr();

		int32 OutSize = 0;
		RecvSocket->SetReceiveBufferSize(2048, OutSize);

		// Bind to our listen port
		if (!RecvSocket->Bind(*Addr))
		{
			SocketSubsystem->DestroySocket(RecvSocket);
			RecvSocket = nullptr;
			UE_LOG(LogStepVrStreamer, Warning, TEXT("Failed to bind to the listen port (%s) for LiveLink face AR receiving with error (%s)"),
				*Addr->ToString(true), SocketSubsystem->GetSocketError());
		}
	}



	return RecvSocket != nullptr;
}

FLiveLinkBaseStaticData* FStepListenerToSelf::GetStaticData(const FName& FaceID)
{
	auto TempPtr = BaseStaticDatas.Find(FaceID);

	return TempPtr;
}

FLiveLinkBaseFrameData* FStepListenerToSelf::GetFrameData(const FName& FaceID)
{
	auto TempPtr = BaseFrameDatas.Find(FaceID);

	return TempPtr;
}

void FStepListenerToSelf::SetFaceScale(const FName& FaceID, float NewScale)
{
	BaseScale.FindOrAdd(FaceID) = NewScale;
}

void FStepListenerToSelf::InitLiveLinkSource()
{

}
