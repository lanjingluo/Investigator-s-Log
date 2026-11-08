#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/ILPlayerModel.h"
#include "Data/ILDataTypes.h"
#include "ILGameSessionSubsystem.generated.h"

/** 会话层广播委托集合（UI 只订阅，数据流单向）。 */

/** HP/SAN/剩余步数变化（HUD 常驻刷新源）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FILOnStatChanged, int32, CurrentHP, int32, CurrentSAN, int32, RemainingSteps);

/** 获得线索（附带当前信息完整度）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FILOnClueCollected, FName, ClueId, float, CompletionRate);

/** 对话推进到新节点。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FILOnDialogueNodeChanged, FName, NodeId);

/** 周目结束（进入结算，参数为结局 ID）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FILOnRunFinished, FName, EndingId);

/** 请求播放骰子检定动画（检定面板消费）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FILOnDiceRollRequested, const FILDiceRoll&, Roll);

/** 骰子动画播完、检定后果已应用的确认信号。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FILOnDiceRollResolved);

class UILGameSettings;

/**
 * 会话服务中枢：所有玩家状态修改的唯一入口，修改后统一广播委托。
 *
 * 关键约束：UI 与表现层只订阅委托 + 只读查询，不得直接改写玩家模型，
 * 保证数据流单向（UI 可整体替换为 Debug 文本版）；
 * 昏迷隐藏机制（HP 归零自动记账）在此统一触发，游玩过程不产生任何提示。
 */
UCLASS()
class UILGameSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ---- 生命周期 ----

	/** 缓存全局设置并初始化随机流。 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ---- 只读查询 ----

	/**
	 * 取玩家模型（只读）。UI 只允许经此查询，不得缓存引用长期使用。
	 * @return 玩家模型只读引用；GameInstance 类型异常时返回静态空模型防崩
	 */
	const FILPlayerModel& GetModel() const;

	/**
	 * 取玩家当前技能值（未按难度折算）。
	 * @param Skill 目标技能
	 * @return 技能值 1-100
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	int32 GetPlayerSkillValue(EILSkill Skill) const;

	/**
	 * 取掷骰随机流（供对话运行器执行 d100；种子随存档保存以保证可复现）。
	 * @return 随机流引用
	 */
	FRandomStream& GetRollStream();

	// ---- 状态修改（唯一入口）----

	/**
	 * 施加 HP 变化；归零瞬间自动触发昏迷隐藏记账（次数+1、浪费小时数累加）。
	 * @param Delta 变化量，可为负
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void ApplyHPDelta(int32 Delta);

	/**
	 * 施加 SAN 变化（归零仅钳制，癫狂表现由剧情层决定）。
	 * @param Delta 变化量，可为负
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void ApplySANDelta(int32 Delta);

	/**
	 * 获得线索并广播（附带按当前场景线索总数计算的信息完整度）。
	 * @param ClueId 线索 ID（CL-01 等）
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void GrantClue(FName ClueId);

	/**
	 * 尝试消耗步数（自由探索段恒成功且不计数）。
	 * @param ActionType 动作类型（当前仅用于日志归类）
	 * @param Cost 消耗步数，默认 1
	 * @return 步数充足返回 true；不足返回 false，调用方应阻止交互
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool TryConsumeStep(EILActionType ActionType, int32 Cost = 1);

	/**
	 * 进入场景：按场景行配置步数段并刷新场景/线索计数。
	 * @param SceneRow 场景表行（FILSceneRow）
	 * @param SceneId 场景 ID（写入玩家模型当前场景）
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void BeginScene(const FILSceneRow& SceneRow, FName SceneId);

	/** 设置剧情标记位。 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SetFlag(FName Flag);

	/** 查询剧情标记位。 */
	UFUNCTION(BlueprintPure, Category = "Session")
	bool HasFlag(FName Flag) const;

	/** 增加未分配技能点（检定成功/支线/发现真相的奖励入口）。 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void AddSkillPoints(int32 Points);

	/** 结束周目，广播结算事件（参数为结局 ID）。 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FinishRun(FName EndingId);

	/**
	 * 取当前周目信息完整度（0-1）。
	 * @return 已收集线索数 / 当前场景线索总数；线索总数为 0 时返回 0
	 */
	float GetCompletionRate() const
	{
		return GetModel().Investigation.GetCompletionRate(CurrentTotalClueCount);
	}

	// ---- 广播（表现层只订阅）----

	/** HP/SAN/剩余步数变化。 */
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FILOnStatChanged OnStatChanged;

	/** 获得线索。 */
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FILOnClueCollected OnClueCollected;

	/** 对话推进到新节点。 */
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FILOnDialogueNodeChanged OnDialogueNodeChanged;

	/** 周目结束。 */
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FILOnRunFinished OnRunFinished;

	/** 请求骰子检定动画。 */
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FILOnDiceRollRequested OnDiceRollRequested;

	/** 骰子动画结束确认。 */
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FILOnDiceRollResolved OnDiceRollResolved;

private:
	/** 统一的状态变更广播，避免每个修改入口各写一遍。 */
	void BroadcastStats();

	/** 取可写玩家模型（内部修改专用；GameInstance 类型异常时为静态空模型）。 */
	FILPlayerModel& GetModelMutable();

	/** 全局可调参数缓存（避免每次 GetDefault 查找）。 */
	UPROPERTY()
	TObjectPtr<UILGameSettings> CachedSettings;

	/** 掷骰随机流；种子随存档保存以保证同存档结果可复现。 */
	FRandomStream RollStream;

	/** 当前场景线索总数（BeginScene 时从场景行记录，用于信息完整度）。 */
	int32 CurrentTotalClueCount = 0;
};
