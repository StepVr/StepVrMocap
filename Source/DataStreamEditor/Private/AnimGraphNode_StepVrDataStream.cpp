// Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.


#include "AnimGraphNode_StepVrDataStream.h"
#include "Kismet2/CompilerResultsLog.h"
#include "AnimationGraphSchema.h"

#include "StepMocapDefine.h"
#include "StepVrSkt.h"

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
	return FText::FromString("Connect streamed skeletal animation from Step");
}

FString UAnimGraphNode_StepDataStream::GetNodeCategory() const
{
	return FString("StepVr");
}

void UAnimGraphNode_StepDataStream::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
	if (Node.SktName.IsEmpty())
	{
		MessageLog.Note(TEXT("Skt Empty, Use BindMocapBones|BindMocapHandBones!"));
		
		TArray<FString> NoteMessage;
		for (auto& BoneName : StepBoneNames)
		{
			FBoneReference* Ref = Node.BindMocapBones.Find(BoneName);
			if (Ref == nullptr)
			{
				NoteMessage.Add(BoneName);
				continue;
			}
			if (!Ref->Initialize(ForSkeleton))
			{
				NoteMessage.Add(BoneName);
			}
		}

		if (NoteMessage.IsValidIndex(0))
		{
			MessageLog.Warning(TEXT("StepMocap Bind Error"));
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
			if (!Ref->Initialize(ForSkeleton))
			{
				NoteMessage.Add(BoneName);
			}
		}

		if (NoteMessage.IsValidIndex(0))
		{
			MessageLog.Warning(TEXT("StepHandCapture Bind Error"));
			for (auto& BoneName : NoteMessage)
			{
				FString Message = FString::Printf(TEXT("Can Not Find Bone %s"), *BoneName);
				MessageLog.Note(*Message);
			}
		}
	}
	else
	{
		TArray<FString> NoteMessage;
		MessageLog.Note(*FString::Printf(TEXT("Use Skt : StepVrMocap/ThirdParty/skt/%s.txt"), *Node.SktName));

		auto ArySkt = STEPVRSKT->GetSktRetarget(Node.SktName);
		if (ArySkt.Num() == (STEPHANDBONESNUMS + STEPBONESNUMS))
		{
			for (int Index = 0; Index < ArySkt.Num(); Index++)
			{
				if (Index == 0)
				{
					continue;
				}

				if (ArySkt[Index].IsEmpty())
				{
					continue;
				}

				if (ArySkt[Index].Equals(TEXT("None")))
				{
					continue;
				}

				FBoneReference Ref(*ArySkt[Index]);
				if (!Ref.Initialize(ForSkeleton))
				{
					FString Message = FString::Printf(TEXT("Use Skt, Skt->%s Bind Error"), *ArySkt[Index]);
					NoteMessage.Add(Message);
				}
			}

			for (auto& temp : NoteMessage)
			{
				MessageLog.Note(*temp);
			}
			if (NoteMessage.IsValidIndex(0))
			{
				MessageLog.Warning(TEXT("Use Skt, Skt Error!"));
			}
			else
			{
				MessageLog.Note(TEXT("Use Skt, Skt Success!"));
			}
		}
		else
		{
			MessageLog.Warning(TEXT("Use Skt, Skt Error!"));
		}
		
	}
}

void UAnimGraphNode_StepDataStream::ValidateAnimNodePostCompile(FCompilerResultsLog& MessageLog, UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex)
{

}

void UAnimGraphNode_StepDataStream::CreateOutputPins()
{
	CreatePin(EGPD_Output, UAnimationGraphSchema::PC_Struct, FComponentSpacePoseLink::StaticStruct(), TEXT("Pose"));
}
