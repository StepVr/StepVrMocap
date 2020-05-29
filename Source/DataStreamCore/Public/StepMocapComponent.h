// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StepMocapComponent.generated.h"


/*DECLARE_EVENT_OneParam*/




//控制数据流
UCLASS(ClassGroup = StepvrClassGroup, meta = (BlueprintSpawnableComponent))
class STEPVRDATASTREAMCORE_API UStepMocapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStepMocapComponent();
	~UStepMocapComponent();


	UFUNCTION(BlueprintCallable)
	void UpdateSkt();
	UFUNCTION(BlueprintPure)
	FString GetSktName();

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

	UFUNCTION(BlueprintPure)
	void MocapDataState(bool& Server, bool& Body, bool& Hand);

protected:
	bool IsRecord = false;

	FString strRecordName;
};

