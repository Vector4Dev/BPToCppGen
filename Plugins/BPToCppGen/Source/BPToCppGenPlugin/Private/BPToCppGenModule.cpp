#include "BPToCppGenModule.h"
#include "SBPToCppGenPanel.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "ToolMenus.h"
#include "LevelEditor.h"

const FName FBPToCppGenModule::BPToCppGenTabName(TEXT("BPToCppGen"));

void FBPToCppGenModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(BPToCppGenTabName, FOnSpawnTab::CreateRaw(this, &FBPToCppGenModule::OnSpawnPluginTab))
		.SetDisplayName(NSLOCTEXT("BPToCppGen", "TabTitle", "BP to C++ Generator"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBPToCppGenModule::RegisterMenus));
}

void FBPToCppGenModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BPToCppGenTabName);
}

void FBPToCppGenModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& Section = Menu->FindOrAddSection("VectorFourStudiosTools");

	Section.AddMenuEntry(
		"OpenBPToCppGen",
		NSLOCTEXT("BPToCppGen", "MenuTitle", "BP to C++ Generator"),
		NSLOCTEXT("BPToCppGen", "MenuTooltip", "Paste a Blueprint graph description and generate a matching C++ class skeleton"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(FBPToCppGenModule::BPToCppGenTabName);
		}))
	);
}

TSharedRef<SDockTab> FBPToCppGenModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SBPToCppGenPanel)
		];
}

IMPLEMENT_MODULE(FBPToCppGenModule, BPToCppGenPlugin)
