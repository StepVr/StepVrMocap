#include "StepMocapComponent.h"
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
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	SetIsReplicated(false);

}

void UStepMocapComponent::UpdateSkt()
{
	FString OwnerName = GetOwner()->GetName();

	
	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(OwnerName);
	if (Stream.IsValid())
	{
		FMocapServerInfo* ServerInfo = Stream->HasActorName(OwnerName);
		if (ServerInfo)
		{
			Stream->ReplcaeSkt(ServerInfo->SktName);
		}
	}
}

UStepMocapComponent::~UStepMocapComponent()
{
	StopRecord();
}

void UStepMocapComponent::StartRecord(const FString& RecordName)
{
	if (IsRecord)
	{
		return;
	}

	IsRecord = true;

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
	if (IsRecord == false)
	{
		return;
	}
	IsRecord = false;

	FString OwnerName = GetOwner()->GetName();

	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(OwnerName);
	if (Stream.IsValid())
	{
		Stream->RecordStop();
	}

	FTimerHandle TimerHandle;
	FString RecordFileName = strRecordName;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [RecordFileName]()
		{
			IPlatformFile& FilePtr = FPlatformFileManager::Get().GetPlatformFile();

			//转移
			FString SavePath = FPaths::ProjectSavedDir() + TEXT("StepRecord/") + RecordFileName;
			if (!FPaths::DirectoryExists(SavePath))
			{
				FilePtr.CreateDirectoryTree(*SavePath);
			}

			FString StartTPose = TEXT("C:\\StepVR_MMAP\\param\\tposData.bin");
			FString StartRecord = TEXT("C:\\StepVR_MMAP\\Record\\") + RecordFileName + TEXT(".mop");

			FString EndTPose = SavePath + TEXT("\\tposData.bin");
			FString EndRecord = SavePath + TEXT("\\Data.mop");

			FilePtr.CopyFile(*EndTPose, *StartTPose);
			FilePtr.CopyFile(*EndRecord, *StartRecord);
		} ,1.f,false,1.f);

}

void UStepMocapComponent::PlayRecord(const FString& RecordName)
{

}

void UStepMocapComponent::GetAllRecords(TArray<FString>& RecordName)
{

}

void UStepMocapComponent::TPose()
{
	FString OwnerName = GetOwner()->GetName();

	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(OwnerName);
	if (Stream.IsValid())
	{
		Stream->TPose();
	}
}

bool UStepMocapComponent::IsMocapReplicate()
{
	return bMocapReplicate;
}
