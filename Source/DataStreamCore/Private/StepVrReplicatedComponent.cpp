// Fill out your copyright notice in the Description page of Project Settings.

#include "StepVrReplicatedComponent.h"
#include "GameFramework/Pawn.h"
#include "StepVrGlobal.h"
#include "StepVrServerModule.h"
#include "Net/UnrealNetwork.h"



// Sets default values for this component's properties
UStepReplicatedComponent::UStepReplicatedComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bReplicates = true;

	// ...
}


void UStepReplicatedComponent::SetPlayerAddrOnServer_Implementation(const uint32 InAddr)
{
	PlayerAddr = InAddr;
}

bool UStepReplicatedComponent::SetPlayerAddrOnServer_Validate(const uint32 InAddr)
{
	return true;
}

// Called when the game starts
void UStepReplicatedComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn == nullptr)
	{
		return;
	}

	bIsLocalControll = Pawn->IsLocallyControlled();
	if (bIsLocalControll)
	{
		if (STEPVR_SERVER_IsValid)
		{
			uint32 Addr = STEPVR_SERVER->GetLocalAddress();
			SetPlayerAddrOnServer(Addr);
		}
		else
		{
			SetPlayerAddrOnServer(1);
		}
	}
	
}
void UStepReplicatedComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStepReplicatedComponent, PlayerAddr);
}

