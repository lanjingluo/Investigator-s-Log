#include "UI/ILDiceRollWidget.h"

#include "Components/TextBlock.h"
#include "Core/ILCheckSystem.h"
#include "Subsystems/ILDialogueRunner.h"
#include "TimerManager.h"
#include "Types/SlateEnums.h"

void UILDiceRollWidget::PlayRoll(const FILDiceRoll& Roll, UILDialogueRunner* Runner)
{
	RunnerPtr = Runner;
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// 判定公式：「侦查 65 → 掷出 42，成功」——全部要素来自检定快照，不在 UI 里重复判定
	if (RollText)
	{
		RollText->SetText(FText::Format(
			NSLOCTEXT("IL", "DiceRollFmt", "{0} {1} → 掷出 {2}，{3}"),
			UILCheckSystem::GetSkillText(Roll.Skill),
			FText::AsNumber(Roll.EffectiveTarget),
			FText::AsNumber(Roll.RawRoll),
			UILCheckSystem::GetResultText(Roll.Result)));
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RollTimerHandle, this, &UILDiceRollWidget::FinishRoll, RollDuration, false);
	}
}

void UILDiceRollWidget::HidePanel()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RollTimerHandle);
	}
	SetVisibility(ESlateVisibility::Collapsed);
}

void UILDiceRollWidget::FinishRoll()
{
	RollTimerHandle.Invalidate();
	SetVisibility(ESlateVisibility::Collapsed);

	// 把推进权交还运行器：应用 SAN/HP/线索/技能点并跳转到成功或失败节点
	if (UILDialogueRunner* Runner = RunnerPtr.Get())
	{
		Runner->ResolveRoll();
	}
}

void UILDiceRollWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RollTimerHandle);
	}
	Super::NativeDestruct();
}
