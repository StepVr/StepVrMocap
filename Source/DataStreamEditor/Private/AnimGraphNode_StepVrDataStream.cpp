// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.


#include "AnimGraphNode_StepVrDataStream.h"
#include "Kismet2/CompilerResultsLog.h"

UAnimGraphNode_StepDataStream::UAnimGraphNode_StepDataStream(const FObjectInitializer&  PCIP)
	: Super(PCIP)
{
}

FText UAnimGraphNode_StepDataStream::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("Step Stream");
}

FLinearColor UAnimGraphNode_StepDataStream::GetNodeTitleColor() const
{
	return FLinearColor(0.75f, 0.75f, 0.75f);
}

FText UAnimGraphNode_StepDataStream::GetTooltipText() const
{
	return FText::FromString("Retrieves streamed skeletal animation from Step");
}

FString UAnimGraphNode_StepDataStream::GetNodeCategory() const
{
	return FString("IKINEMA");
}

void UAnimGraphNode_StepDataStream::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
}