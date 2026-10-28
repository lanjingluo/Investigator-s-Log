#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ILGameSettings.generated.h"

/**
 * 项目级可调参数，暴露在 Project Settings → Game → Investigator's Log。
 *
 * 存在意义：判定边界与步数池公式属于数值调优高频项（策划原文的步数池公式
 * 与「常见 15-25 步」描述矛盾，大成功规则为程序侧补全），做成 Config=Game
 * 的 DeveloperSettings 后，改参数写回 DefaultGame.ini，无需重编译。
 * Core 层不直接读本类，由上层读出后以参数注入，保证 Core 可独立测试。
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Investigator's Log"))
class UILGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ---- 检定规则 ----

	/** 是否启用大成功规则（策划只定义了大失败分支，大成功为程序侧补全，可关）。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Check")
	bool bUseCriticalSuccess = true;

	/** 大成功的最大点数（策划定义 01-05）。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Check", meta = (ClampMin = "1", ClampMax = "20"))
	int32 CriticalSuccessMax = 5;

	/** 大失败的最小点数（策划定义 96-100）。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Check", meta = (ClampMin = "80", ClampMax = "100"))
	int32 CriticalFailureMin = 96;

	// ---- 时间/步数 ----

	/** 步数池基数（策划原文 12 + INT 修正，按「常见 15-25 步」校准为 15）。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Time")
	int32 StepBudgetBase = 15;

	/** 步数池 INT 除数：Bonus = Floor((INT - 10) / Divisor)，越小步数池越大。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Time")
	float StepBudgetIntDivisor = 1.5f;

	/** 步数池下界。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Time")
	int32 StepBudgetMin = 8;

	/** 步数池上界。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Time")
	int32 StepBudgetMax = 25;

	/** 每次昏迷浪费的游戏内小时数（隐藏机制，仅结算界面展示）。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Time")
	int32 WastedHoursPerFaint = 2;

	// ---- 成长 ----

	/** 新周目初始技能点。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 InitialSkillPoints = 0;
};
