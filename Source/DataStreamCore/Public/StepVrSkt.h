// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
#pragma once
#include "CoreMinimal.h"

typedef TMap<FString, TArray<FString>> SktFiles;

#define STEPVRSKT FStepVrSkt::GetInstance()
class STEPVRDATASTREAMCORE_API FStepVrSkt
{
public:
	static FStepVrSkt* GetInstance();

	void LoadSkt();
	void AddFiles(FString& FileName, TArray<FString>& SkeletonID);
	TArray<FString>& GetSktRetarget(FString& FileName);

private:
	void AppendSkt(FString& FileName, TArray<FString>& OutData);

	SktFiles AllFiles;
};