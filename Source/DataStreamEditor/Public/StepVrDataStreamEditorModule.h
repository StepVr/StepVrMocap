//* Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.

#pragma once

#include "Modules/ModuleInterface.h"


DECLARE_LOG_CATEGORY_EXTERN(LogStepDataStreamEditor, Warning, All);

class FStepDataStreamEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
