// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StepVrReplicatedComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UStepReplicatedComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStepReplicatedComponent();

	//本地IpAddress
	UFUNCTION(Server, Reliable, WithValidation)
	void SetPlayerAddrOnServer(const uint32 InAddr);

	UPROPERTY(Replicated)
	uint32  PlayerAddr = 0;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;		
	
	bool bIsLocalControll = false;
};
