#pragma once

#include "CoreMinimal.h"
#include "Core/ILTypes.h"
#include "UI/ILUserWidgetBase.h"
#include "ILDiceRollWidget.generated.h"

class UILDialogueRunner;
class UTextBlock;

/**
 * 骰子检定面板：展示判定公式「技能 目标值 → 掷出 点数，结果」。
 * 动画由 WBP 增强；C++ 侧用定时器保证无动画资产时也能按固定时长推进——
 * 时长到点后调用运行器 ResolveRoll() 应用结果并分叉剧情。
 */
UCLASS()
class UILDiceRollWidget : public UILUserWidgetBase
{
	GENERATED_BODY()

public:
	/**
	 * 弹出检定面板并启动计时。
	 * @param Roll    检定快照（含技能、目标值、掷出点数与结论）
	 * @param Runner  对话运行器（弱引用；到点后调用其 ResolveRoll）
	 */
	void PlayRoll(const FILDiceRoll& Roll, UILDialogueRunner* Runner);

	/** 收起面板并取消未触发的计时。 */
	void HidePanel();

protected:
	virtual void NativeDestruct() override;

	/** 展示时长到点：收起面板并通知运行器应用检定后果。 */
	void FinishRoll();

	/** 判定公式文本。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RollText;

	/** 展示时长（秒），为 WBP 骰子动画预留窗口。 */
	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (ClampMin = "0.1"))
	float RollDuration = 1.2f;

private:
	/** 到点结算计时器。 */
	FTimerHandle RollTimerHandle;

	/** 对话运行器弱引用。 */
	TWeakObjectPtr<UILDialogueRunner> RunnerPtr;
};
