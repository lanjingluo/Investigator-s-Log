#pragma once

#include "CoreMinimal.h"
#include "ILPartyState.generated.h"

/**
 * 单个队友的运行时状态。
 * 覆盖策划要求的三种走向：崩溃（SAN 归零）、背叛（剧情标记）、牺牲/离队；
 * 隐藏秘密是否揭开独立标记，供对话系统做「摊牌」分支。
 */
USTRUCT(BlueprintType)
struct FILPartyMemberState
{
	GENERATED_BODY()

	/** 队友标识，与对话表 SpeakerId 对应（如 "Margaret"、"Elias"、"Ruth"）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FName MemberId;

	/** 当前 HP。 */
	UPROPERTY(BlueprintReadWrite, Category = "Party")
	int32 CurrentHP = 10;

	/** 最大 HP。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 MaxHP = 10;

	/** 当前 SAN。 */
	UPROPERTY(BlueprintReadWrite, Category = "Party")
	int32 CurrentSAN = 30;

	/** 最大 SAN。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 MaxSAN = 30;

	/** SAN 归零后的癫狂状态（幂等：只置一次）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Party")
	bool bIsBroken = false;

	/** 是否已触发背叛剧情（伊莱亚斯线转折）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Party")
	bool bIsBetrayed = false;

	/** 隐藏秘密是否已被玩家揭开。 */
	UPROPERTY(BlueprintReadOnly, Category = "Party")
	bool bSecretRevealed = false;

	/** 是否已牺牲/离队；离队后不再出现在对话与助战中。 */
	UPROPERTY(BlueprintReadOnly, Category = "Party")
	bool bIsGone = false;

	/**
	 * 施加 HP 变化，结果 clamp 到 [0, MaxHP]。
	 * 队友 HP 归零不进入昏迷流程（与主角不同），由上层按剧情标记 bIsGone。
	 * @param Delta 变化量，可为负
	 */
	void ApplyHP(int32 Delta);

	/**
	 * 施加 SAN 变化，结果 clamp 到 [0, MaxSAN]；归零时置 bIsBroken = true（幂等）。
	 * @param Delta 变化量，可为负
	 */
	void ApplySAN(int32 Delta);
};
