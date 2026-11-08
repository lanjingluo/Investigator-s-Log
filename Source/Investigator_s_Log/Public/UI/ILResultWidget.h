#pragma once

#include "CoreMinimal.h"
#include "UI/ILUserWidgetBase.h"
#include "ILResultWidget.generated.h"

class UTextBlock;

/**
 * 结算界面：结局名 + 隐藏机制点破。
 * 策划约束：昏迷次数与被浪费的时间只在结算时展示——本界面是唯一读取
 * FILTimeState::FaintCount / WastedHours 的 UI。
 */
UCLASS()
class UILResultWidget : public UILUserWidgetBase
{
	GENERATED_BODY()

public:
	/**
	 * 显示结算。
	 * @param EndingId 结局 ID（由 FinishRun 广播）
	 */
	void ShowResult(FName EndingId);

protected:
	/** 结局名。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EndingNameText;

	/** 昏迷次数（隐藏机制点破）。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FaintCountText;

	/** 因昏迷浪费的游戏内小时数。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WastedTimeText;

	/** 信息完整度百分比。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CompletionRateText;
};
