// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.


#include "AnimGraphNode_StepVrDataStream.h"
#include "Kismet2/CompilerResultsLog.h"
#include "StepMocapDefine.h"


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
	return FString("StepVr");
}

void UAnimGraphNode_StepDataStream::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
	TArray<FString> NoteMessage;
	for (auto& BoneName : StepBoneNames)
	{
		FBoneReference* Ref = Node.BindMocapBones.Find(BoneName);
		if (Ref == nullptr)
		{
			NoteMessage.Add(BoneName);
			continue;
		}

		Ref->Initialize(ForSkeleton);

		if (!Ref->HasValidSetup())
		{
			NoteMessage.Add(BoneName);
		}
	}

	if (NoteMessage.IsValidIndex(0))
	{
		MessageLog.Warning(TEXT("StepMocap Bind Error,Not working properly"));
		for (auto& BoneName : NoteMessage)
		{
			FString Message = FString::Printf(TEXT("Can Not Find Bone %s"), *BoneName);
			MessageLog.Note(*Message);
		}
	}

	NoteMessage.Empty();
	for (auto& BoneName : StepHandBoneNames)
	{
		FBoneReference* Ref = Node.BindMocapHandBones.Find(BoneName);
		if (Ref == nullptr)
		{
			NoteMessage.Add(BoneName);
			continue;
		}
		Ref->Initialize(ForSkeleton);

		if (!Ref->HasValidSetup())
		{
			NoteMessage.Add(BoneName);
		}
	}

	if (NoteMessage.IsValidIndex(0))
	{
		MessageLog.Warning(TEXT("StepHandCapture Bind Error,Not working properly"));
		for (auto& BoneName : NoteMessage)
		{
			FString Message = FString::Printf(TEXT("Can Not Find Bone %s"), *BoneName);
			MessageLog.Note(*Message);
		}
	}



}