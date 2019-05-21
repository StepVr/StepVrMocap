// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StepVrReplicatedComponent.generated.h"

#define REPLICATE_NONE TEXT("None")

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UStepReplicatedComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStepReplicatedComponent();

	//本地IpAddress
	UFUNCTION(Server, Reliable, WithValidation)
	void SetPlayerAddrOnServer(const FString& InAddr);

	UPROPERTY(BlueprintReadOnly, transient, Replicated)
	FString PlayerAddr;

	UFUNCTION(BlueprintCallable)
	void SetPlayerAddr(const FString& InAddr);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;		
	
	bool bIsLocalControll = false;
};
