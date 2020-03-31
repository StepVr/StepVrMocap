#include "StepVrSkt.h"
#include "StepMocapDefine.h"


#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"


static FStepVrSkt GStepVrSkt;
FStepVrSkt* FStepVrSkt::GetInstance()
{
	return &GStepVrSkt;
}

void FStepVrSkt::LoadSkt()
{
	class FSktFiles : public IPlatformFile::FDirectoryVisitor
	{
	public:
		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			FString Filename(FilenameOrDirectory);
			
			if (!bIsDirectory && Filename.EndsWith(".txt"))
			{
				TArray<FString> TempArray;
				FFileHelper::LoadFileToStringArray(TempArray,*Filename);

				int32 char1, char2;
				if (Filename.FindLastChar('/', char1) && Filename.FindLastChar('.', char2))
				{
					FString SubFileName = Filename.Mid(char1 + 1, char2 - char1 -1);
					STEPVRSKT->AddFiles(SubFileName, TempArray);
				}
			}

			return true;
		}
	};

	AllFiles.Empty();
	FSktFiles SktFiles;
	FString SktPath = FPaths::ProjectPluginsDir().Append("/StepVrMocap/ThirdParty/skt");
	FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*SktPath, SktFiles);
}


void FStepVrSkt::AddFiles(FString& FileName, TArray<FString>& SkeletonID)
{
	AllFiles.FindOrAdd(FileName) = SkeletonID;
}

TArray<FString>& FStepVrSkt::GetSktRetarget(FString& FileName)
{
	static TArray<FString> TempPtr;

	auto Ptr = AllFiles.Find(FileName);
	if (Ptr == NULL)
	{
		TempPtr.Empty();
		AppendSkt(FileName, TempPtr);
	}
	else
	{
		return *Ptr;
	}

	return TempPtr;
}

bool FStepVrSkt::ReplcaeSkt(const FString& NewSkt)
{
	FString TargetPath = TEXT("C:\\StepVR_MMAP\\param\\NewModel_humanoid.skt");
	FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*TargetPath);

	//还原skt
	if (NewSkt.IsEmpty())
	{
		return true;
	}

	//替换skt
	FString SktPath = FPaths::ProjectPluginsDir().
		Append("/StepVrMocap/ThirdParty/skt/").
		Append(NewSkt).Append(".skt");

	if (!FPaths::FileExists(SktPath))
	{
		UE_LOG(LogStepMocap, Log, TEXT("ReplcaeSkt Error,%s cant find"), *SktPath);
		return false;
	}

	return FPlatformFileManager::Get().GetPlatformFile ().CopyFile(*TargetPath, *SktPath);
}

void FStepVrSkt::AppendSkt(FString& FileName, TArray<FString>& OutData)
{
	FString SktPath = FPaths::ProjectPluginsDir().
		Append("/StepVrMocap/ThirdParty/skt/").
		Append(FileName).Append(".txt");

	if (FPaths::FileExists(SktPath))
	{
		OutData.Empty();
		FFileHelper::LoadFileToStringArray(OutData, *SktPath);

		AddFiles(FileName, OutData);
	}
}

