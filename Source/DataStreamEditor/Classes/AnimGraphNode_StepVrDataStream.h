// Copyright (C) 2006-2016, IKinema Ltd. All rights reserved.

#pragma once

#include "AnimGraphNode_Base.h"
#include "AnimNode_StepVrDataStream.h"
#include "AnimGraphNode_StepVrDataStream.generated.h"

UCLASS(MinimalAPI)
class UAnimGraphNode_StepDataStream : public UAnimGraphNode_Base
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
	FAnimNode_StepDataStream Node;

public:
	// UEdGraphNode interface
	FText			GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FLinearColor	GetNodeTitleColor() const override;
	FText			GetTooltipText() const override;
	FString			GetNodeCategory() const override;
	void			ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog) override;	
	// Create any output pins necessary for this node
	virtual void	CreateOutputPins() override;
	// End of UEdGraphNode interface
};


