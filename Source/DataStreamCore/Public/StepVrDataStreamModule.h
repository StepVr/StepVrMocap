#pragma once
#include "Modules/ModuleManager.h"

class FStepDataStreamModule : public IModuleInterface
{

public:
	void StartupModule() override;
	void ShutdownModule() override;
};
