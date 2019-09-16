#include "StepVrDataServer.h"



StepVrDataServer::StepVrDataServer()
{
}


StepVrDataServer::~StepVrDataServer()
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
