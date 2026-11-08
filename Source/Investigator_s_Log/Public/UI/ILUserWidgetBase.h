#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ILUserWidgetBase.generated.h"

class UILGameSessionSubsystem;

/**
 * 所有 UMG 界面的 C++ 基类。
 * 统一提供会话子系统访问与 UI 输入模式切换；子类一律使用 meta=(BindWidget) 绑定控件，
 * 控件缺失时编译期报错而非运行期崩溃。
 * 订阅约定：子类重写 SubscribeSession/UnsubscribeSession 挂接会话委托，
 * 父类在 NativeConstruct/NativeDestruct 中成对调用，避免遗漏取消订阅。
 */
UCLASS(Abstract)
class UILUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 取会话子系统（经所属 PlayerController 的 GameInstance 查找）。
	 * @return 会话子系统；GameInstance 类型异常时返回 nullptr，调用方需判空
	 */
	UFUNCTION(BlueprintPure, Category = "UI")
	UILGameSessionSubsystem* GetSession() const;

	/**
	 * 设置 UI 输入模式；弹窗打开时传 true 以暂停角色移动（转发给玩家控制器统一实现）。
	 * @param bUIOnly true 为仅 UI 模式，false 为 Game+UI 并行
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetUIInputMode(bool bUIOnly);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 子类重写以订阅会话委托；父类不自动订阅，避免无谓开销。 */
	virtual void SubscribeSession();

	/** 子类重写以取消订阅（与 SubscribeSession 严格成对）。 */
	virtual void UnsubscribeSession();
};
