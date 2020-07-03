#include "StepMocapComponent.h"
#include "StepVrStream.h"


#include "GameFramework/Actor.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Components/ActorComponent.h"



UStepMocapComponent::UStepMocapComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	SetIsReplicated(false);
}

void UStepMocapComponent::UpdateSkt()
{
	uint32 UID = GetOwner()->GetUniqueID();

	
	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(UID);
	if (Stream.IsValid())
	{
		FMocapServerInfo* ServerInfo = Stream->HasActorWithUID(UID);
		if (ServerInfo)
		{
			Stream->ReplcaeSkt(ServerInfo->SktName);
		}
	}
}

FString UStepMocapComponent::GetSktName()
{
	uint32 UID = GetOwner()->GetUniqueID();

	FString SktName = "";
	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(UID);
	if (Stream.IsValid())
	{
		FMocapServerInfo* ServerInfo = Stream->HasActorWithUID(UID);
		if (ServerInfo)
		{
			SktName = ServerInfo->SktName;
		}
	}

	return SktName;
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

	//UID
	uint32 UID = GetOwner()->GetUniqueID();

	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(UID);
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

	uint32 UID = GetOwner()->GetUniqueID();

	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(UID);
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
	uint32 UID = GetOwner()->GetUniqueID();

	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(UID);
	if (Stream.IsValid())
	{
		Stream->TPose();
	}
}

void UStepMocapComponent::MocapDataState(bool& Server, bool& Body, bool& Hand)
{
	uint32 UID = GetOwner()->GetUniqueID();

	TSharedPtr<FStepMocapStream> Stream = FStepMocapStream::GetActorMocapStream(UID);
	if (Stream.IsValid())
	{
		Server = Stream->IsConnected();
		Body = Stream->IsBodyConnect();
		Hand = Stream->IsHandConnect();
	}
}
