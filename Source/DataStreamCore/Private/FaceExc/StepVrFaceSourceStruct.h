#pragma once
#include "StepVrFaceSourceStruct.generated.h"

USTRUCT()
struct FStepFaceData 
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TMap<FString,float> data;

	UPROPERTY()
	FString timestamp;

	UPROPERTY()
	FString socket_id;
};
