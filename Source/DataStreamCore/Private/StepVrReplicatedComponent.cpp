// Fill out your copyright notice in the Description page of Project Settings.

#include "StepVrReplicatedComponent.h"
#include "StepMocapDefine.h"

#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"




// Sets default values for this component's properties
UStepReplicatedComponent::UStepReplicatedComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bReplicates = true;

	PlayerAddr = REPLICATE_NONE;
	// ...
}

void UStepReplicatedComponent::SetPlayerAddr(const FString& InAddr)
{
	SetPlayerAddrOnServer(InAddr);
}

//FString UStepReplicatedComponent::GetClientLocalIP()
//{
//	FString NewIP = GetClientLocalIP_BP();
//
//	if (NewIP.IsEmpty())
//	{
//		NewIP = GetLocalIP();
//	}
//
//	return;
//}
//
//FString UStepReplicatedComponent::GetClientLocalIP_BP()
//{
//
//}


void UStepReplicatedComponent::SetPlayerAddrOnServer_Implementation(const FString& InAddr)
{
	PlayerAddr = InAddr;
}

bool UStepReplicatedComponent::SetPlayerAddrOnServer_Validate(const FString& InAddr)
{
	return true;
}

// Called when the game starts
void UStepReplicatedComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn == nullptr || Pawn->Controller == nullptr)
	{
		return;
	}

	APlayerController* LocalController = UGameplayStatics::GetPlayerController(Pawn, 0);
	if (LocalController == nullptr)
	{
		return;
	}

	bIsLocalControll = Cast<AController>(LocalController) == Pawn->Controller;

	if (bIsLocalControll)
	{
		//PlayerAddr = GetClientLocalIP()
		//SetPlayerAddrOnServer(PlayerAddr);
	}
}
void UStepReplicatedComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStepReplicatedComponent, PlayerAddr);
}

