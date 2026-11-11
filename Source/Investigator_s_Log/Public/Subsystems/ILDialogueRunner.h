#pragma once

#include "CoreMinimal.h"
#include "Data/ILDataTypes.h"
#include "ILDialogueRunner.generated.h"

class UDataTable;
class UILGameSessionSubsystem;

/**
 * 对话运行器：驱动数据表对话分支的状态机（进入节点 → 选项/检定 → 分叉 → 收敛）。
 *
 * 职责边界：只做节点推进与规则调用，不认识任何 Widget——骰子动画通过
 * Session 的 OnDiceRollRequested 委托交给 UI，动画结束由 UI 回调 ResolveRoll 继续，
 * 保证表现层可整体替换；副作用（线索/SAN/HP/标记/技能点）全部经会话子系统入口。
 */
UCLASS()
class UILDialogueRunner : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 绑定数据源与会话服务（GameMode 每关装配时调用一次）。
	 * @param InDialogueTable 对话表（行结构 FILDialogueNode）
	 * @param InClueTable 线索表（行结构 FILClueRow，供扩展校验）
	 * @param InSession 会话子系统
	 */
	void Initialize(UDataTable* InDialogueTable, UDataTable* InClueTable, UILGameSessionSubsystem* InSession);

	/** 从指定节点开始对话。 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartAt(FName NodeId);

	/** 线性节点推进到 NextNodeId（带选项的节点必须显式选择，推进被忽略）。 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Advance();

	/**
	 * 选择第 ChoiceIndex 个选项（下标对应 GetAvailableChoices 结果）。
	 * 校验顺序：等待骰子中 → 选项可用性 → 步数充足；需要检定时掷骰并等待动画。
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectChoice(int32 ChoiceIndex);

	/** 骰子动画播完后由 UI 调用：应用检定结果并跳转成功/失败分支。 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ResolveRoll();

	/** 是否正在等待骰子动画（期间禁止再次选择）。 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsWaitingForRoll() const { return bWaitingForRoll; }

	/**
	 * 取当前节点数据。
	 * @param OutNode 输出节点副本
	 * @return 节点存在返回 true
	 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool GetCurrentNode(FILDialogueNode& OutNode) const;

	/** 当前节点的选项列表副本（未过滤，配合 IsChoiceEnabled 做置灰）。 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	TArray<FILDialogueChoice> GetAvailableChoices() const;

	/**
	 * 选项当前是否可选（前置线索/标记/一次性检查）。
	 * @param Choice 目标选项
	 * @return 可选返回 true；不满足则 UI 置灰
	 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsChoiceEnabled(const FILDialogueChoice& Choice) const;

	/**
	 * 选项检定成功率（悬停 tooltip 显示用），与 RollCheck 判定边界一致。
	 * @param Choice 目标选项
	 * @return 0.0-1.0
	 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	float GetChoiceSuccessChance(const FILDialogueChoice& Choice) const;

	/**
	 * 从存档恢复运行器状态（GameMode 装配 Runner 后、对话开始前调用）。
	 * @param InUsedChoiceIds 已使用的一次性选项集合
	 * @param InSeed 检定随机流种子
	 */
	void RestoreState(const TSet<FName>& InUsedChoiceIds, int32 InSeed);

	/** 检定随机流当前种子（存档时取走用于复现）。 */
	int32 GetRollSeed() const { return RollStream.GetCurrentSeed(); }

	/** 已使用的一次性选项集合只读视图（存档时取走）。 */
	const TSet<FName>& GetUsedChoiceIds() const { return UsedChoiceIds; }

private:
	/**
	 * 进入节点：应用副作用（必得线索/SAN/HP/标记/技能点/检查点）后广播节点变更；
	 * 节点缺失或 None 视为对话结束，仅广播不报错。
	 */
	void EnterNode(FName NodeId);

	/** 按检定结果跳转成功/失败节点（大成功/大失败特殊剧情待表数据就绪后接入）。 */
	void ApplyRollOutcome(EILRollResult Result);

	/** 对话表（行结构 FILDialogueNode）。 */
	UPROPERTY()
	TObjectPtr<UDataTable> DialogueTable;

	/** 线索表（行结构 FILClueRow）。 */
	UPROPERTY()
	TObjectPtr<UDataTable> ClueTable;

	/** 会话子系统（副作用与广播的唯一通道）。 */
	UPROPERTY()
	TObjectPtr<UILGameSessionSubsystem> Session;

	/** 当前节点 ID；None 表示对话未开始或已结束。 */
	FName CurrentNodeId;

	/** 是否等待骰子动画播完。 */
	bool bWaitingForRoll = false;

	/** 待结算检定快照（UI 动画展示用）。 */
	FILDiceRoll PendingRoll;

	/** 待跳转的成功节点。 */
	FName PendingSuccessNodeId;

	/** 待跳转的失败节点。 */
	FName PendingFailureNodeId;

	/** 已使用的一次性选项（bOnceOnly 置灰依据，须随存档持久化）。 */
	TSet<FName> UsedChoiceIds;

	/** 对话检定独立随机流（与会话掷骰隔离，便于单独复现）。 */
	FRandomStream RollStream;
};
