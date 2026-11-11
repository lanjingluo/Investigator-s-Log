#pragma once

#include "CoreMinimal.h"
#include "Core/ILPlayerModel.h"
#include "GameFramework/SaveGame.h"
#include "ILSaveGame.generated.h"

/**
 * 周目存档：完整快照玩家模型、检查点与检定随机流状态。
 * SaveVersion 不匹配的旧档在加载时直接弃用，防止结构漂移导致脏数据。
 */
UCLASS()
class UILSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** 当前代码期望的存档版本；与档内 SaveVersion 不符时拒绝加载。 */
	static constexpr int32 CurrentSaveVersion = 1;

	/** 构造（字段默认值在头文件内联，保留给后续版本迁移钩子）。 */
	UILSaveGame();

	/** 存档版本号（写入 CurrentSaveVersion，加载时校验）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	int32 SaveVersion = CurrentSaveVersion;

	/** 周目玩家模型完整快照（属性/HP/SAN/线索/时间线/标记/技能点/场景）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FILPlayerModel PlayerModel;

	/** 当前检查点节点 ID（读档后从此节点恢复对话）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FName CurrentCheckpointNodeId;

	/** 检定随机流种子（读档后注入运行器，保证检定序列可复现）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	int32 RandomSeed = 0;

	/** 已使用的一次性选项集合（防止读档后重复刷检定拿技能点）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	TSet<FName> UsedChoiceIds;
};
