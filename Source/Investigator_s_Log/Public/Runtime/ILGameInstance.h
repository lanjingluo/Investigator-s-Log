#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Core/ILPlayerModel.h"
#include "ILGameInstance.generated.h"

class UILGameSessionSubsystem;

/**
 * 全局游戏实例：持有单周目玩家状态（唯一权威数据）与当前检查点。
 * 跨关卡不销毁，因此是周目数据的合法宿主；GameMode 每关重建，不持有周目数据。
 * 所有修改应经由 UILGameSessionSubsystem 入口，以保证委托广播一致。
 */
UCLASS(Blueprintable)
class UILGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/** 单周目玩家状态（唯一权威数据）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	FILPlayerModel PlayerModel;

	/** 当前所在检查点节点 ID（存档/读档后从此节点恢复对话）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	FName CurrentCheckpointNodeId;

	/** 重置为新周目：清空模型、按属性回满、按全局配置给初始技能点。 */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void ResetRun();

	/** 取会话子系统的快捷方式。 */
	UFUNCTION(BlueprintPure, Category = "Run")
	UILGameSessionSubsystem* GetSession() const;

	/**
	 * 切换场景：记录目标场景 ID 并输出日志；
	 * 场景表查询与关卡无缝跳转在步骤 6 接入 DialogueRunner 后完善。
	 * @param SceneId 场景 ID（S1-1 等）
	 */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void LoadScene(FName SceneId);

	/** 是否存在存档（主菜单/继续按钮开关用）。 */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasSaveRun() const;

	/** 保存当前周目到固定槽位（模型+检查点+检定种子+一次性选项）。 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveRun();

	/** 从固定槽位读取周目（恢复模型/检查点/待恢复状态，HUD 随后刷新）。 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadRun();

protected:
	virtual void Init() override;

public:
	/** 玩家模型只读访问（供会话子系统查询）。 */
	const FILPlayerModel& GetModel() const { return PlayerModel; }

	/** 玩家模型可写访问（仅会话子系统修改入口使用）。 */
	FILPlayerModel& GetModelMutable() { return PlayerModel; }

	/** 读档暂存的一次性选项集合（GameMode 装配 Runner 时取走）。 */
	const TSet<FName>& GetPendingUsedChoiceIds() const { return PendingUsedChoiceIds; }

	/** 读档暂存的检定随机流种子（GameMode 装配 Runner 时取走）。 */
	int32 GetPendingRollSeed() const { return PendingRollSeed; }

	/** 是否有读档待恢复的运行器状态（取走后清除，防止重复注入）。 */
	bool HasPendingRunnerState() const { return bHasPendingRunnerState; }

	/** 清除待恢复标记（Runner 注入完成后调用）。 */
	void ClearPendingRunnerState() { bHasPendingRunnerState = false; }

private:
	/** 读档暂存：Runner 每关新建，其状态在 GameMode 装配时注入。 */
	TSet<FName> PendingUsedChoiceIds;

	/** 读档暂存的检定种子。 */
	int32 PendingRollSeed = 0;

	/** 是否存在待注入的运行器状态。 */
	bool bHasPendingRunnerState = false;
};
