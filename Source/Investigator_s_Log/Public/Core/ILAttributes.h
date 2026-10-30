#pragma once

#include "CoreMinimal.h"
#include "Core/ILTypes.h"
#include "ILAttributes.generated.h"

/**
 * 调查员的 8 项基础属性（COC 式，取值 3-18）。
 * 派生数值（最大 HP、最大 SAN、步数池）由只读方法实时计算而非缓存，
 * 避免属性变更后派生值失效；公式参数由调用方注入，便于策划调参与测试。
 */
USTRUCT(BlueprintType)
struct FILAttributes
{
	GENERATED_BODY()

	/** 力量：影响最大 HP。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "3", ClampMax = "18"))
	int32 STR = 10;

	/** 体质：影响最大 HP。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "3", ClampMax = "18"))
	int32 CON = 10;

	/** 敏捷：先攻与闪避相关（战斗模块使用）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "3", ClampMax = "18"))
	int32 DEX = 10;

	/** 外貌：交涉类互动的软性加成（预留）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "3", ClampMax = "18"))
	int32 APP = 10;

	/** 智力：决定限步探索段的步数池。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "3", ClampMax = "18"))
	int32 INT = 10;

	/** 意志：决定最大 SAN。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "3", ClampMax = "18"))
	int32 POW = 10;

	/** 教育：职业技能基础（预留）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "3", ClampMax = "18"))
	int32 EDU = 10;

	/** 幸运：极端结果判定（预留）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ClampMin = "3", ClampMax = "18"))
	int32 LUK = 10;

	/** 最大生命值 = (STR + CON) / 2 + 2，整数除法向下取整；归零昏迷而非死亡。 */
	int32 GetMaxHP() const;

	/** 最大理智值 = POW × 3；归零癫狂。 */
	int32 GetMaxSAN() const;

	/**
	 * 限步探索段的步数池。
	 * 策划原文「12 + INT 修正」在 INT=18 时仅得 16，与「常见 15-25 步」描述不符，
	 * 故公式做成可配置：Base + Floor((INT - 10) / Divisor)，再 clamp 到 [MinValue, MaxValue]。
	 * @param Base      步数基数
	 * @param Divisor   INT 除数，越小步数池越大；传入近零值时按 0 修正处理，防除零
	 * @param MinValue  下界
	 * @param MaxValue  上界
	 * @return 步数池大小
	 */
	int32 GetStepBudget(int32 Base = 15, float Divisor = 1.5f, int32 MinValue = 8, int32 MaxValue = 25) const;
};

/**
 * 技能值集合。
 * 键为 8 项技能，值为 1-100 的最终技能值（已含技能点投入）；
 * 使用 TMap 便于表驱动与存档，未配置的技能返回默认基础值。
 */
USTRUCT(BlueprintType)
struct FILSkillSet
{
	GENERATED_BODY()

	/** 技能值表。未配置的技能视为 DefaultValue。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
	TMap<EILSkill, int32> Values;

	/**
	 * 取技能值。
	 * @param Skill         目标技能
	 * @param DefaultValue  未配置时的回退值
	 * @return 技能值
	 */
	int32 GetSkill(EILSkill Skill, int32 DefaultValue = 5) const;

	/**
	 * 累加技能点（成长系统入口），结果 clamp 到 [0, MaxValue]。
	 * @param Skill    目标技能
	 * @param Delta    增量，可为负
	 * @param MaxValue 技能值上限
	 */
	void AddSkillPoints(EILSkill Skill, int32 Delta, int32 MaxValue = 100);
};
