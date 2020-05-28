// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StepMocapComponent.generated.h"


/*DECLARE_EVENT_OneParam*/




//控制数据流
UCLASS(ClassGroup = StepvrClassGroup, BlueprintType, meta = (BlueprintSpawnableComponent))
class STEPVRDATASTREAMCORE_API UStepMocapComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	~UStepMocapComponent();


	UFUNCTION(BlueprintCallable)
	void UpdateSkt();

	UFUNCTION(BlueprintCallable)
	void StartRecord(const FString& RecordName);
	UFUNCTION(BlueprintCallable)
	void StopRecord();
	UFUNCTION(BlueprintCallable)
	void PlayRecord(const FString& RecordName);
	UFUNCTION(BlueprintCallable)
	void GetAllRecords(TArray<FString>& RecordName);
	UFUNCTION(BlueprintCallable)
	void TPose();

	bool IsMocapReplicate();

protected:
	UPROPERTY(BlueprintReadWrite)
	bool bMocapReplicate = false;

	bool IsRecord = false;

	FString strRecordName;
};

