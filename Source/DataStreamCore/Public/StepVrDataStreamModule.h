#pragma once
#include "Modules/ModuleManager.h"


class FStepListenerToAppleARKit;
class FStepDataStreamModule : public IModuleInterface
{

public:
	void StartupModule() override;
	void ShutdownModule() override;

	static FStepListenerToAppleARKit* GetStepListenerToAppleARKit();
};
