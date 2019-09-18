#include "StepVrDataServer.h"





/************************************************************************/
/*						StepMagic  Start                                */
/************************************************************************/
#ifdef WITH_STEPMAGIC
class FStepMagicData : public StepVrDataServer
{
public:
	~FStepMagicData();

	virtual void Connect2Server(const FString& IP, int32 port) override;
	virtual void DisConnect() override;

	virtual bool HasBodyData() override;
	virtual void GetBodyData(TArray<FTransform>& OutData) override;

	virtual bool HasHandData() override;
	virtual void GetHandData(TArray<FRotator>& OutData) override;

	virtual bool HasFaceData() override;
	virtual void GetFaceData(TMap<FString, float>& OutData) override;
};
FStepMagicData::~FStepMagicData()
{

}

void FStepMagicData::Connect2Server(const FString& IP, int32 port)
{

}

void FStepMagicData::DisConnect()
{

}

bool FStepMagicData::HasBodyData()
{

}

void FStepMagicData::GetBodyData(TArray<FTransform>& OutData)
{

}

bool FStepMagicData::HasHandData()
{

}

void FStepMagicData::GetHandData(TArray<FRotator>& OutData)
{

}

bool FStepMagicData::HasFaceData()
{

}

void FStepMagicData::GetFaceData(TMap<FString, float>& OutData)
{

}
#endif // WITH_STEPMAGIC

/************************************************************************/
/*						StepMagic  End                                  */
/************************************************************************/


/************************************************************************/
/*						MMAP Start                                      */
/************************************************************************/

//MMAP
class FServicesData : public StepVrDataServer
{
public:
	~FServicesData();

	virtual void Connect2Server(const FString& IP, int32 port) override;
	virtual void DisConnect() override;

	virtual bool HasBodyData() override;
	virtual void GetBodyData(TArray<FTransform>& OutData) override;

	virtual bool HasHandData() override;
	virtual void GetHandData(TArray<FRotator>& OutData) override;

	virtual bool HasFaceData() override;
	virtual void GetFaceData(TMap<FString, float>& OutData) override;
};

FServicesData::~FServicesData()
{

}

bool FServicesData::HasBodyData()
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}

bool FServicesData::HasHandData()
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}

bool FServicesData::HasFaceData()
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}

void FServicesData::Connect2Server(const FString& IP, int32 port)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

void FServicesData::DisConnect()
{
	//throw std::logic_error("The method or operation is not implemented.");
}

void FServicesData::GetBodyData(TArray<FTransform>& OutData)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

void FServicesData::GetHandData(TArray<FRotator>& OutData)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

void FServicesData::GetFaceData(TMap<FString, float>& OutData)
{
	//throw std::logic_error("The method or operation is not implemented.");
}


/************************************************************************/
/*                          MMAP End                                    */
/************************************************************************/

StepVrDataServer::StepVrDataServer()
{
}


StepVrDataServer::~StepVrDataServer()
{
}

TSharedPtr<StepVrDataServer> StepVrDataServer::CreateServerData()
{
#ifdef WITH_STEPMAGIC
	return MakeShareable(new FStepMagicData());
#else
	return MakeShareable(new FServicesData()); 
#endif // WITH_STEPMAGIC
}



















