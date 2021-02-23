#pragma once
#include "ARTrackable.h"


struct FLiveLinkBaseStaticData;
struct FLiveLinkBaseFrameData;



class FStepListenerToSelf :
	public FTickableGameObject
{
public:
	static FStepListenerToSelf* CreateRemoteListener();

	FStepListenerToSelf();
	virtual ~FStepListenerToSelf();

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	
	virtual bool IsTickable() const override 
	{ 
		return RecvSocket != nullptr; 
	}
	
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FStepListenerToSelf, STATGROUP_Tickables);
	}

	virtual bool IsTickableInEditor() const override { return true; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	// End FTickableGameObject

	bool InitReceiveSocket();

	FLiveLinkBaseStaticData* GetStaticData(const FName& FaceID);
	FLiveLinkBaseFrameData* GetFrameData(const FName& FaceID);

	//面部进行缩放
	void SetFaceScale(const FName& FaceID, float NewScale);

private:
	void InitLiveLinkSource();

	/** Socket that is read from to publish remote data to this local LiveLinkClient */
	FSocket*			RecvSocket;
	/** Buffer to reuse for receiving data */
	TArray<uint8>		RecvBuffer;
	/** The reused blend shape map to avoid allocs/frees */
	FARBlendShapeMap	BlendShapes;

	//FLiveLinkBaseStaticData		BaseStaticData;
	//FLiveLinkBaseFrameData		BaseFrameData;

	TMap<FName, FLiveLinkBaseStaticData> BaseStaticDatas;
	TMap<FName, FLiveLinkBaseFrameData> BaseFrameDatas;
	/** The source used to publish events from the remote socket */
	//TSharedPtr<ILiveLinkSourceARKit> Source;

	TMap<FName, float>	BaseScale;
};