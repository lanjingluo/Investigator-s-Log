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

protected:
	virtual void Init() override;

public:
	/** 玩家模型只读访问（供会话子系统查询）。 */
	const FILPlayerModel& GetModel() const { return PlayerModel; }

	/** 玩家模型可写访问（仅会话子系统修改入口使用）。 */
	FILPlayerModel& GetModelMutable() { return PlayerModel; }
};
