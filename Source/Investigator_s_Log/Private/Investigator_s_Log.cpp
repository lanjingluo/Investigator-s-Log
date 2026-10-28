#include "Investigator_s_Log.h"

#include "Modules/ModuleManager.h"

void FInvestigatorModule::StartupModule()
{
	// 先调用父类，保证 FDefaultGameModuleImpl 的默认初始化流程不被跳过
	FDefaultGameModuleImpl::StartupModule();
}

void FInvestigatorModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
}

// 使用自定义模块类替换 FDefaultGameModuleImpl，从而获得 Startup/Shutdown 重写能力
IMPLEMENT_PRIMARY_GAME_MODULE(FInvestigatorModule, Investigator_s_Log, "Investigator_s_Log");
