#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/ILTypes.h"
#include "ILCheckSystem.generated.h"

/**
 * d100 百分骰检定的无状态工具集。
 * 全部函数为纯函数（除显式传入的随机流外无副作用），可在 Automation 测试中直接调用；
 * UI 层（检定面板）只消费 FILDiceRoll 快照与预估值，不重复实现判定规则。
 */
UCLASS()
class UILCheckSystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 按难度折算目标值：普通 = 原值，困难 = 原值/2，极难 = 原值/5，均向下取整。
	 * @param SkillValue 原始技能值（1-100）
	 * @param Difficulty 检定难度
	 * @return 折算后的目标值，最小为 1（保证极低技能值仍有成功可能）
	 */
	UFUNCTION(BlueprintPure, Category = "Check")
	static int32 GetEffectiveTarget(int32 SkillValue, EILDifficulty Difficulty);

	/**
	 * 预估成功率，供对话选项悬停 tooltip 显示。
	 * 与 RollCheck 判定边界严格一致：大失败区间（>= CriticalFailureMin）恒为失败，
	 * 大成功是成功的子集，不影响成功率计数。
	 * @param SkillValue          原始技能值
	 * @param Difficulty          检定难度
	 * @param bUseCriticalSuccess 是否启用大成功规则（仅用于语义对齐，成功率不受影响）
	 * @param CriticalFailureMin  大失败最小点数（默认 96）
	 * @return 0.0-1.0 的成功概率
	 */
	UFUNCTION(BlueprintPure, Category = "Check")
	static float GetSuccessChance(int32 SkillValue, EILDifficulty Difficulty,
		bool bUseCriticalSuccess = true, int32 CriticalFailureMin = 96);

	/**
	 * 执行一次 d100 检定。
	 * 判定顺序：大失败优先（即使目标值很高，96-100 依旧失败，保证「永远有失败可能」）→
	 * 大成功（需同时满足低点数与本身确实成功，避免极难检定被大成功规则白送）→ 常规比较。
	 * @param SkillValue          原始技能值
	 * @param Difficulty          检定难度
	 * @param Stream              随机流引用；种子随存档保存即可复现
	 * @param OutRoll             输出：本次检定的完整快照（Skill 字段由调用方覆盖）
	 * @param bUseCriticalSuccess 是否启用大成功规则
	 * @param CriticalSuccessMax  大成功最大点数（默认 5，即 01-05）
	 * @param CriticalFailureMin  大失败最小点数（默认 96，即 96-100）
	 * @return 判定结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Check")
	static EILRollResult RollCheck(int32 SkillValue, EILDifficulty Difficulty,
		UPARAM(ref) FRandomStream& Stream, FILDiceRoll& OutRoll,
		bool bUseCriticalSuccess = true,
		int32 CriticalSuccessMax = 5,
		int32 CriticalFailureMin = 96);

	/** 结果枚举转中文显示文本（UI 标签类文案，允许放 C++）。 */
	UFUNCTION(BlueprintPure, Category = "Check")
	static FText GetResultText(EILRollResult Result);

	/** 技能枚举转中文显示名，供选项徽标拼装「[侦查·困难]」。 */
	UFUNCTION(BlueprintPure, Category = "Check")
	static FText GetSkillText(EILSkill Skill);
};
