#include "StepMocapInterface.h"
#include "StepMocapDefine.h"
#include "StepVrStream.h"


TSharedPtr<FStepMocapStream> GetStepMocapStreams(const FString& Serverip)
{
	FMocapServerInfo ServerInfo;
	ServerInfo.ServerIP = Serverip;

	return FStepMocapStream::GetStepMocapStream(ServerInfo, false);
}

UStepMocapInterface::UStepMocapInterface(const FObjectInitializer& Obj)
	:Super(Obj)
{

}

void IStepMocapInterface::UseCurrentSkt()
{
	auto MocapStream = GetStepMocapStreams(ServerIP);

	if (MocapStream.IsValid())
	{
		MocapStream->ReplcaeSkt(SktName);
	}
}
