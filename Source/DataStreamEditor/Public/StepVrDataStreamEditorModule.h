//* Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.

#pragma once
#include "Modules/ModuleManager.h"

class FStepDataStreamEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
