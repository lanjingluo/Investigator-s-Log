#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * 项目主模块实现类。
 *
 * 继承 FDefaultGameModuleImpl 以保留 Game 模块的默认行为，同时补上
 * StartupModule / ShutdownModule 重写点：后续在编辑器启动期注册 Slate 样式、
 * 资产类型动作（AssetTypeActions）或加载默认数据资产时，必须经由这两个入口，
 * 而引擎默认的 FDefaultGameModuleImpl 不提供这些虚函数。
 */
class FInvestigatorModule : public FDefaultGameModuleImpl
{
public:
	/** 模块加载时调用。当前仅透传父类初始化，后续在此注册编辑器扩展与默认配置。 */
	virtual void StartupModule() override;

	/** 模块卸载时调用。必须成对释放 StartupModule 中注册的资源，避免编辑器热重载残留。 */
	virtual void ShutdownModule() override;
};
