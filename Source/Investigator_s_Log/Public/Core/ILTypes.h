#pragma once

#include "CoreMinimal.h"
#include "ILTypes.generated.h"

/**
 * 本作裁剪后的 8 项技能（COC 式技能池）。
 * 枚举顺序仅用于显示排序，逻辑一律按键值引用，不依赖序号。
 */
UENUM(BlueprintType)
enum class EILSkill : uint8
{
	/** 侦查：搜查现场、观察细节。 */
	Investigation	UMETA(DisplayName = "侦查"),
	/** 心理学：读人、识破情绪与谎言。 */
	Psychology		UMETA(DisplayName = "心理学"),
	/** 说服：交涉、问询、谈判。 */
	Persuasion		UMETA(DisplayName = "说服"),
	/** 神秘学：识别符文、禁书、仪式知识。 */
	Occultism		UMETA(DisplayName = "神秘学"),
	/** 图书馆使用：查档案、翻账本、检索资料。 */
	LibraryUse		UMETA(DisplayName = "图书馆使用"),
	/** 攀爬：翻墙、攀岩、撬高处暗门。 */
	Climb			UMETA(DisplayName = "攀爬"),
	/** 潜行：避开巡逻、藏身暗处。 */
	Stealth			UMETA(DisplayName = "潜行"),
	/** 格斗：近身冲突（本作轻战斗，仅保底解法）。 */
	Fighting		UMETA(DisplayName = "格斗")
};

/**
 * 检定难度。
 * 困难 = 目标值 ÷2，极难 = 目标值 ÷5，均向下取整，最小为 1。
 */
UENUM(BlueprintType)
enum class EILDifficulty : uint8
{
	/** 普通：按技能值原样判定。 */
	Regular	UMETA(DisplayName = "普通"),
	/** 困难：目标值减半。 */
	Hard	UMETA(DisplayName = "困难"),
	/** 极难：目标值除以五。 */
	Extreme	UMETA(DisplayName = "极难")
};

/**
 * 单次 d100 检定的四态结果。
 * 大成功/大失败会触发特殊剧情分支，其余二态走常规成功/失败分支。
 */
UENUM(BlueprintType)
enum class EILRollResult : uint8
{
	CriticalSuccess	UMETA(DisplayName = "大成功"),
	Success			UMETA(DisplayName = "成功"),
	Failure			UMETA(DisplayName = "失败"),
	CriticalFailure	UMETA(DisplayName = "大失败")
};

/**
 * 探索动作类型。
 * 仅用于限步段的步数消耗归类与 HUD 提示文案；自由探索段不消耗步数。
 */
UENUM(BlueprintType)
enum class EILActionType : uint8
{
	Move	UMETA(DisplayName = "移动"),
	Search	UMETA(DisplayName = "搜查"),
	Ask		UMETA(DisplayName = "问询")
};

/**
 * 一次 d100 检定的完整快照。
 * 供检定面板播放骰子动画并渲染判定公式（「技能值 → 掷出点数，结果」）；
 * 由 UILCheckSystem::RollCheck 填充，调用方只需覆盖 Skill 字段。
 */
USTRUCT(BlueprintType)
struct FILDiceRoll
{
	GENERATED_BODY()

	/** 本次检定使用的技能（调用方覆盖）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Check")
	EILSkill Skill = EILSkill::Investigation;

	/** 本次检定的难度。 */
	UPROPERTY(BlueprintReadOnly, Category = "Check")
	EILDifficulty Difficulty = EILDifficulty::Regular;

	/** 参与比较的原始技能值（未按难度折算）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Check")
	int32 RawSkillValue = 0;

	/** 按难度折算后的目标值，掷骰需 ≤ 该值才算成功。 */
	UPROPERTY(BlueprintReadOnly, Category = "Check")
	int32 EffectiveTarget = 0;

	/** 实际掷出的 d100 点数（1-100）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Check")
	int32 RawRoll = 0;

	/** 判定结论。 */
	UPROPERTY(BlueprintReadOnly, Category = "Check")
	EILRollResult Result = EILRollResult::Failure;

	/** 判定是否成功（大成功与普通成功均为 true）。 */
	UFUNCTION(BlueprintPure, Category = "Check")
	bool IsSuccess() const
	{
		return Result == EILRollResult::Success || Result == EILRollResult::CriticalSuccess;
	}
};
