#pragma once

#include "CoreMinimal.h"
#include "ILTimeState.generated.h"

/**
 * 时间与步数状态，同时是「昏迷隐藏机制」的唯一数据源。
 *
 * 关键约束：该结构只记录数值，绝不产生任何 UI 提示——
 * 策划要求昏迷的时间代价在游玩过程中不可见，只有通关结算界面才允许读取
 * FaintCount / WastedHours 点破隐藏机制。
 */
USTRUCT(BlueprintType)
struct FILTimeState
{
	GENERATED_BODY()

	/** 当前场景的步数池；0 表示当前段不限步。 */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 StepBudget = 0;

	/** 已消耗步数（仅限步段累计）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 StepsSpent = 0;

	/** 当前场景是否为限步段（决定 HUD 是否显示剩余步数）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	bool bLimitedSteps = false;

	/** 累计昏迷次数，终章难度加成的直接来源。 */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 FaintCount = 0;

	/** 因昏迷累计浪费的游戏内小时数，仅在结算界面展示。 */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 WastedHours = 0;

	/**
	 * 剩余步数。
	 * @return 限步段返回 StepBudget - StepsSpent；自由段返回 -1 表示无限
	 */
	int32 GetRemainingSteps() const;

	/**
	 * 进入新场景时重置步数段。
	 * @param InBudget 步数池；<= 0 视为自由探索段
	 */
	void BeginScene(int32 InBudget);

	/**
	 * 尝试消耗步数。自由探索段（bLimitedSteps=false）恒成功且不计数。
	 * @param Cost 本次消耗步数，默认 1（移动/搜查/问询各消耗 1 步）
	 * @return 步数充足返回 true；不足返回 false，调用方应阻止该交互
	 */
	bool TrySpendStep(int32 Cost = 1);

	/**
	 * 记录一次昏迷（HP 归零时由上层调用）。只累加计数与浪费时间，不触发任何提示。
	 * @param WastedHoursPerFaint 每次昏迷浪费的小时数（由 UILGameSettings 注入）
	 */
	void ApplyFaint(int32 WastedHoursPerFaint = 2);

	/** 终章事件难度加成 = 昏迷次数（策划公式：难度 = 基础 + 昏迷次数）。 */
	int32 GetFinaleDifficultyBonus() const { return FaintCount; }
};
