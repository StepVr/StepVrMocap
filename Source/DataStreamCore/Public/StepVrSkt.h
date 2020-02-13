#pragma once
#include "CoreMinimal.h"

typedef TMap<FString, TArray<FString>> SktFiles;

#define STEPVRSKT FStepVrSkt::GetInstance()
class STEPVRDATASTREAMCORE_API FStepVrSkt
{
public:
	static FStepVrSkt* GetInstance();

	//加载所有SKT
	void LoadSkt();

	//存储单个SKTName
	void AddFiles(FString& FileName, TArray<FString>& SkeletonID);

	//获取skt对应的骨骼名
	TArray<FString>& GetSktRetarget(FString& FileName);

	//替换当前Skt
	bool ReplcaeSkt(const FString& NewSkt);

private:
	void AppendSkt(FString& FileName, TArray<FString>& OutData); 

	SktFiles AllFiles;
};