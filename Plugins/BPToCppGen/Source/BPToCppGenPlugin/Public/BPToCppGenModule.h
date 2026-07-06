#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FBPToCppGenModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	void RegisterMenus();

	static const FName BPToCppGenTabName;
};
