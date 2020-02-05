// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "StepMocapInterface.generated.h"

UINTERFACE(MinimalAPI)
class UStepMocapInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()

};

class IStepMocapInterface 
{
	GENERATED_IINTERFACE_BODY()

public:
	//使用当前skt
	virtual void UseCurrentSkt();
	

	FString ServerIP;

	FString SktName;


};
