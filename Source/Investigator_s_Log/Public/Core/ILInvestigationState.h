#pragma once

#include "CoreMinimal.h"
#include "ILInvestigationState.generated.h"

/**
 * 线索收集状态。
 * 键为线索 ID（CL-01 ... CL-30），值为是否已获得；
 * 线索的分级（关键/可选）、描述与证据图等静态数据存于线索表（FILClueRow），此处只管运行时状态。
 */
USTRUCT(BlueprintType)
struct FILInvestigationState
{
	GENERATED_BODY()

	/** 已获得线索集合。value 恒为 true，保留 bool 以便未来支持「部分线索」语义。 */
	UPROPERTY(BlueprintReadOnly, Category = "Clues")
	TMap<FName, bool> ClueCollected;

	/** 获得线索；重复获得保持幂等（值保持 true）。 */
	void AddClue(FName ClueId);

	/** 是否已获得指定线索。 */
	bool HasClue(FName ClueId) const;

	/** 已获得线索数量。 */
	int32 GetCollectedCount() const;

	/**
	 * 信息完整度（0-1），用于结算界面展示。
	 * 关键线索与可选线索等权计分：二者共同决定结局质量的可及范围。
	 * @param TotalClueCount 本周目线索总数（由线索表统计）；<= 0 时返回 0，避免除零
	 * @return 完整度比例，clamp 到 [0, 1]
	 */
	float GetCompletionRate(int32 TotalClueCount) const;
};
