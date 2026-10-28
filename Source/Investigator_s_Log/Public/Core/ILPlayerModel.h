#pragma once

#include "CoreMinimal.h"
#include "Core/ILAttributes.h"
#include "Core/ILTimeState.h"
#include "Core/ILInvestigationState.h"
#include "Core/ILPartyState.h"
#include "ILPlayerModel.generated.h"

/**
 * 单周目玩家状态的聚合根，也是存档（UILSaveGame）的唯一序列化来源。
 *
 * 设计约束：
 * - 只包含 USTRUCT/POD 数据，不含任何 Actor / Widget / World 引用，
 *   保证可被纯逻辑层与 Automation 测试直接构造与验证；
 * - 所有修改须经会话子系统（UILGameSessionSubsystem）入口，UI 只读。
 */
USTRUCT(BlueprintType)
struct FILPlayerModel
{
	GENERATED_BODY()

	/** 8 项基础属性。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	FILAttributes Attributes;

	/** 技能值集合。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	FILSkillSet Skills;

	/** 当前 HP；归零时由上层调用 Time.ApplyFaint 走昏迷流程。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	int32 CurrentHP = 10;

	/** 当前 SAN；归零即癫狂（本作主角癫狂等价于坏结局触发条件之一）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	int32 CurrentSAN = 30;

	/** 未分配的技能点（成长系统货币）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	int32 SkillPoints = 0;

	/** 时间/步数/昏迷状态（隐藏机制数据源）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	FILTimeState Time;

	/** 线索收集状态。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	FILInvestigationState Investigation;

	/** 队友状态列表。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	TArray<FILPartyMemberState> Party;

	/** 剧情标记位集合（如 "EliasBetrayed"、"RuthBrotherFound"）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	TSet<FName> Flags;

	/** 当前章节 ID。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	FName CurrentChapterId;

	/** 当前场景 ID（S1-1 等）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Model")
	FName CurrentSceneId;

	/** 新周目初始化：按属性计算 HP/SAN 上限并回满。 */
	UFUNCTION(BlueprintCallable, Category = "Model")
	void InitializeFromAttributes();

	/**
	 * 施加 HP 变化，clamp 到 [0, GetMaxHP()]；归零的昏迷副作用由上层处理。
	 * @param Delta 变化量，可为负
	 */
	UFUNCTION(BlueprintCallable, Category = "Model")
	void ApplyHP(int32 Delta);

	/**
	 * 施加 SAN 变化，clamp 到 [0, GetMaxSAN()]。
	 * @param Delta 变化量，可为负
	 */
	UFUNCTION(BlueprintCallable, Category = "Model")
	void ApplySAN(int32 Delta);

	/**
	 * 按队友 ID 查找队友（非 UFUNCTION，返回裸指针仅限 C++ 调用）。
	 * @param MemberId 队友标识
	 * @return 命中的队友指针；未找到返回 nullptr
	 */
	FILPartyMemberState* FindPartyMember(FName MemberId);
};
