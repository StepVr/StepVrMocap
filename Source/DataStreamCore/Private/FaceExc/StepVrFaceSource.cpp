#include "StepVrFaceSource.h"
#include "SocketSubsystem.h"




const uint32 Max_receive_ARKitMessage = 2048;
const uint32 Min_receive_ARKitMessage = 260;

//const uint32 MAX_BLEND_SHAPE_PACKET_SIZE = sizeof(BLEND_SHAPE_PACKET_VER) + sizeof(FQualifiedFrameTime) + sizeof(uint8) + (sizeof(float) * (uint64)EARFaceBlendShape::MAX) + (sizeof(TCHAR) * 256) + (sizeof(TCHAR) * 256);
//const uint32 MIN_BLEND_SHAPE_PACKET_SIZE = sizeof(BLEND_SHAPE_PACKET_VER) + sizeof(FQualifiedFrameTime) + sizeof(uint8) + (sizeof(float) * (uint64)EARFaceBlendShape::MAX) + sizeof(TCHAR) + sizeof(TCHAR);


static FName ParseEnumName(FName EnumName)
{
	const int32 BlendShapeEnumNameLength = 19;
	FString EnumString = EnumName.ToString();
	return FName(*EnumString.Right(EnumString.Len() - BlendShapeEnumNameLength));
}



FStepListenerToAppleARKit* FStepListenerToAppleARKit::CreateRemoteListener()
{
	static FStepListenerToAppleARKit* Listener = nullptr;
	if (Listener == nullptr)
	{
		Listener = new FStepListenerToAppleARKit();
		if (!Listener->InitReceiveSocket())
		{
			delete Listener;
			Listener = nullptr;
		}
	}

	return Listener;
}


FStepListenerToAppleARKit::FStepListenerToAppleARKit():
	RecvSocket(nullptr)
{
	RecvBuffer.AddUninitialized(Max_receive_ARKitMessage);
}

FStepListenerToAppleARKit::~FStepListenerToAppleARKit()
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

void FStepListenerToAppleARKit::Tick(float DeltaTime)
{
	uint32 BytesPending = 0;
	while (RecvSocket->HasPendingData(BytesPending))
	{
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
		TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();

		int32 BytesRead = 0;
		if (RecvSocket->RecvFrom(RecvBuffer.GetData(), RecvBuffer.Num(), BytesRead, *Sender) &&
			// Make sure the packet is a complete one and ignore if it is not
			BytesRead > Min_receive_ARKitMessage)
		{
			uint8 PacketVer = 0;
			FName SubjectName;
			uint8 BlendShapeCount = (uint8)EARFaceBlendShape::MAX;
			FName DeviceId;
			FQualifiedFrameTime FrameTime;

			FNboSerializeFromBuffer FromBuffer(RecvBuffer.GetData(), BytesRead);

			FromBuffer >> PacketVer;
			if (FromBuffer.HasOverflow())
			{
				UE_LOG(LogStepVrStreamer, Verbose, TEXT("Packet overflow reading the packet version for the face AR packet"));
				return;
			}
			FromBuffer >> DeviceId;
			FromBuffer >> SubjectName;
			FromBuffer >> FrameTime;
			FromBuffer >> BlendShapeCount;

			if (FromBuffer.HasOverflow() || BlendShapeCount != (uint8)EARFaceBlendShape::MAX)
			{
				UE_LOG(LogStepVrStreamer, Verbose, TEXT("Packet overflow reading the face AR packet's non-array fields"));
				return;
			}

			// Loop through and parse each float for each enum
			for (uint8 BlendShapeIndex = 0; BlendShapeIndex < BlendShapeCount && !FromBuffer.HasOverflow(); BlendShapeIndex++)
			{
				float Value = 0.f;
				FromBuffer >> Value;
				BlendShapes.Add((EARFaceBlendShape)BlendShapeIndex, Value);
			}
			// All of the data was valid, so publish it
			if (!FromBuffer.HasOverflow())
			{
				FLiveLinkBaseFrameData& BaseFrameData = BaseFrameDatas.FindOrAdd(SubjectName);
				FLiveLinkBaseStaticData& BaseStaticData = BaseStaticDatas.FindOrAdd(SubjectName);
				float TempScale = 1.f;
				if (auto ScalePtr = BaseScale.Find(SubjectName))
				{
					TempScale = *ScalePtr;
				}

				//缓存数据FrameData
				BaseFrameData.WorldTime = FPlatformTime::Seconds();
				BaseFrameData.MetaData.SceneTime = FrameTime;
				BaseFrameData.PropertyValues.Reset((int32)EARFaceBlendShape::MAX);

				//缓存StaticData
				//Update property names array
				BaseStaticData.PropertyNames.Reset((int32)EARFaceBlendShape::MAX);
				//Iterate through all valid blend shapes to extract names
				const UEnum* EnumPtr = StaticEnum<EARFaceBlendShape>();

				// Iterate through all of the blend shapes copying them into the LiveLink data type
				for (int32 Shape = 0; Shape < (int32)EARFaceBlendShape::MAX; Shape++)
				{
					if(BlendShapes.Contains((EARFaceBlendShape)Shape)) 
					{
						const float CurveValue = BlendShapes.FindChecked((EARFaceBlendShape)Shape) * TempScale;
						BaseFrameData.PropertyValues.Add(FMath::Clamp(CurveValue,0.f,1.f));

						const FName ShapeName = ParseEnumName(EnumPtr->GetNameByValue(Shape));
						BaseStaticData.PropertyNames.Add(ShapeName);
					}
				}
			}
			else
			{
				UE_LOG(LogStepVrStreamer, Verbose, TEXT("Packet overflow reading the face AR packet's array of blend shapes"));
			}
		}
	}
}

bool FStepListenerToAppleARKit::InitReceiveSocket()
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
	TSharedRef<FInternetAddr> Addr = SocketSubsystem->GetLocalBindAddr(*GLog);
	
	Addr->SetPort(11112);

	RecvSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("FStepListenerToAppleARKit socket"), Addr->GetProtocolType());
	if (RecvSocket != nullptr)
	{
		RecvSocket->SetReuseAddr();
		RecvSocket->SetNonBlocking();
		RecvSocket->SetRecvErr();
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

FLiveLinkBaseStaticData* FStepListenerToAppleARKit::GetStaticData(const FName& FaceID)
{
	auto TempPtr = BaseStaticDatas.Find(FaceID);

	return TempPtr;
}

FLiveLinkBaseFrameData* FStepListenerToAppleARKit::GetFrameData(const FName& FaceID)
{
	auto TempPtr = BaseFrameDatas.Find(FaceID);

	return TempPtr;
}

void FStepListenerToAppleARKit::SetFaceScale(const FName& FaceID, float NewScale)
{
	BaseScale.FindOrAdd(FaceID) = NewScale;
}

void FStepListenerToAppleARKit::InitLiveLinkSource()
{

}
