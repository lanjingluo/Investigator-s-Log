#pragma once

#include "CoreMinimal.h"
#include "UI/ILUserWidgetBase.h"
#include "ILHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UWidget;

/**
 * 常驻 HUD：HP 条、SAN 条、剩余步数（仅限步场景显示）。
 * 只订阅 OnStatChanged + 首次显示时按当前模型补齐一次，不主动轮询。
 */
UCLASS()
class UILHUDWidget : public UILUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void SubscribeSession() override;
	virtual void UnsubscribeSession() override;

	/** 首次显示时按模型当前值补齐，避免「建好 UI 后必须等下一次变化才刷新」。 */
	void RefreshFromModel();

	/** 状态变化回调（HP/SAN/剩余步数）。 */
	UFUNCTION()
	void HandleStatChanged(int32 CurrentHP, int32 CurrentSAN, int32 RemainingSteps);

	/** HP 进度条。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPBar;

	/** SAN 进度条。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> SANBar;

	/** 剩余步数容器（自由探索段整体隐藏）。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> StepPanel;

	/** 剩余步数文本。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StepText;

	/** 临时提示文本（如「步数已耗尽」）；可选控件。 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ToastText;
};
