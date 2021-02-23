#pragma once
#include "Modules/ModuleManager.h"


class FStepListenerToAppleARKit;
class FStepListenerToSelf;



class FStepDataStreamModule : public IModuleInterface
{

public:
	void StartupModule() override;
	void ShutdownModule() override;

	static FStepListenerToAppleARKit* GetStepListenerToAppleARKit();
	static FStepListenerToSelf* GetStepListenerToSelf();
};
