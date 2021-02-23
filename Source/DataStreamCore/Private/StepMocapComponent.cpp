#include "StepMocapComponent.h"
#include "AnimNode_StepVrDataStream.h"
#include "StepVrStream.h"


#include "GameFramework/Actor.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Components/ActorComponent.h"


UStepMocapComponent::UStepMocapComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	SetIsReplicated(false);

}

void UStepMocapComponent::MocapUpdateSkt()
{
	Orders.Enqueue(Type_UpdateSkt);
}

UStepMocapComponent::~UStepMocapComponent()
{
	StopRecord();
}

void UStepMocapComponent::MocapStopSkeletonCapture(bool bStop)
{
	EnableStream = bStop;
	Orders.Enqueue(Type_EnableStream);
}

void UStepMocapComponent::MocapRefreshSkeletonScale()
{
	Orders.Enqueue(Type_GetSkeletonScale);
}

void UStepMocapComponent::MocapSetScaleEnable(bool NewState)
{
	EnableSkeletonScale = NewState;
	Orders.Enqueue(Type_EnableSkeletonScale);
}

void UStepMocapComponent::MocapGetSkeletonScale(FVector & Out)
{
	Out = SkeletonScale;
}

void UStepMocapComponent::StartRecord(const FString& RecordName)
{
	if (bRecord)
	{
		return;
	}

	bRecord = true;

	strRecordName = RecordName;
	FString OwnerName = GetOwner()->GetName();

	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(OwnerName);
	if (Stream.IsValid())
	{
		Stream->RecordStart(RecordName);
	}
}

void UStepMocapComponent::StopRecord()
{
	if (bRecord == false)
	{
		return;
	}
	bRecord = false;

	FString OwnerName = GetOwner()->GetName();

	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(OwnerName);
	if (Stream.IsValid())
	{
		Stream->RecordStop();
	}

	//FTimerHandle TimerHandle;
	//FString RecordFileName = strRecordName;
	//GetWorld()->GetTimerManager().SetTimer(TimerHandle, [RecordFileName]()
	//	{
	//		IPlatformFile& FilePtr = FPlatformFileManager::Get().GetPlatformFile();

	//		//转移
	//		FString SavePath = FPaths::ProjectSavedDir() + TEXT("StepRecord/") + RecordFileName;
	//		if (!FPaths::DirectoryExists(SavePath))
	//		{
	//			FilePtr.CreateDirectoryTree(*SavePath);
	//		}

	//		FString StartTPose = TEXT("C:\\StepVR_MMAP\\param\\tposData.bin");
	//		FString StartRecord = TEXT("C:\\StepVR_MMAP\\Record\\") + RecordFileName + TEXT(".mop");

	//		FString EndTPose = SavePath + TEXT("\\tposData.bin");
	//		FString EndRecord = SavePath + TEXT("\\Data.mop");

	//		FilePtr.CopyFile(*EndTPose, *StartTPose);
	//		FilePtr.CopyFile(*EndRecord, *StartRecord);
	//	} ,1.f,false,1.f);

}

void UStepMocapComponent::PlayRecord(const FString& RecordName)
{

}

void UStepMocapComponent::GetAllRecords(TArray<FString>& RecordName)
{

}

void UStepMocapComponent::MocapTPose()
{
	Orders.Enqueue(Type_Tpose);
}

void UStepMocapComponent::MocapSetNewIP(const FString& InNewIP)
{
	NewServerIP = InNewIP;
	Orders.Enqueue(Type_NewIP);
}

void UStepMocapComponent::MocapSetNewFaceID(const FString& InNewFaceID)
{
	NewFaceID = InNewFaceID;
	Orders.Enqueue(Type_NewFaceID);
}

void UStepMocapComponent::MocapSetNewFaceScale(float NewScale)
{
	FaceScale = NewScale;
	Orders.Enqueue(Type_ScaleFace);
}

void UStepMocapComponent::MocapSetEnableFace(bool Enable, EARFaceBlendShape ARFaceBlendShape)
{
	NewFaceState.FindOrAdd(ARFaceBlendShape) = Enable;
	Orders.Enqueue(Type_EnableFace);
}

void UStepMocapComponent::MocapChangeFaceType(EUseFaceTypeEx UseFaceType)
{
	UseFaceTypeEx = UseFaceType;
	Orders.Enqueue(Type_ChangeFace);
}

bool UStepMocapComponent::IsMocapReplicate()
{
	return bMocapReplicate;
}

void UStepMocapComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	do 
	{
		auto Owner = GetOwner();
		if (Owner->GetLocalRole() < ENetRole::ROLE_Authority)
		{
			break;
		}

		EOrderType* Temp = Orders.Peek();
		if (Temp == nullptr)
		{
			break;
		}

		uint32 OwnerGUID = Owner->GetUniqueID();
		auto StepDataStream = FAnimNode_StepDataStream::GetStepDataStream(OwnerGUID);
		if (StepDataStream == nullptr)
		{
			break;
		}

		switch (*Temp)
		{
		case EOrderType::Type_EnableStream:
		{
			StepDataStream->MocapStopSkeletonCapture(EnableStream);
		}
		break;
		case EOrderType::Type_EnableSkeletonScale:
		{
			StepDataStream->MocapEnableSkeletonScale(EnableSkeletonScale);
		}
		break;
		case EOrderType::Type_Tpose:
		{
			StepDataStream->MocapTPose();
		}
		break;
		case EOrderType::Type_UpdateSkt:
		{
			StepDataStream->MocapUpdateSkt();
		}
		break;
		case  EOrderType::Type_NewIP:
		{
			StepDataStream->MocapSetNewIP(NewServerIP);
		}
		break;
		case  EOrderType::Type_NewFaceID:
		{
			StepDataStream->MocapSetNewFaceID(NewFaceID);
		}
		break;		
		case  EOrderType::Type_EnableFace:
		{
			for (auto TemoPair : NewFaceState)
			{
				StepDataStream->MocapSetEnableFace(TemoPair.Key, TemoPair.Value);
			}
			NewFaceState.Empty();
		}
		break;
		case  EOrderType::Type_ScaleFace:
		{
			StepDataStream->MocapSetNewFaceScale(FaceScale);
		}
		break;
		case  EOrderType::Type_ChangeFace:
		{
			StepDataStream->MocapChangeFaceType((EUseFaceType)UseFaceTypeEx);
		}
		break;
		case  EOrderType::Type_GetSkeletonScale:
		{
			SkeletonScale = StepDataStream->MocapGetSkeletonScale();
		}
		break;
		}

		Orders.Pop();
	} while (0);
}
