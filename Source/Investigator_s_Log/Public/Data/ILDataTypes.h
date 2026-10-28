#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "Engine/DataTable.h"
#include "Core/ILTypes.h"
#include "ILDataTypes.generated.h"

class UWorld;

/**
 * 对话选项：一个可点击的分支入口。
 * 内嵌技能检定配置与成功/失败跳转节点，实现「选项标注技能与难度 + 成败分叉」；
 * 选项徽标「[技能·难度]」由 UI 依据 Skill/Difficulty 自动拼装，策划不在 Text 里手写。
 */
USTRUCT(BlueprintType)
struct FILDialogueChoice
{
	GENERATED_BODY()

	/** 选项唯一 ID，仅用于日志、埋点与 bOnceOnly 记账，不参与节点跳转。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	FName ChoiceId;

	/** 选项正文（玩家可见，走本地化）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	FText Text;

	/** 是否需要掷骰；false 时直接跳 SuccessNodeId。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	bool bRequiresCheck = false;

	/** 检定技能（bRequiresCheck = true 时有效）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	EILSkill Skill = EILSkill::Investigation;

	/** 检定难度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	EILDifficulty Difficulty = EILDifficulty::Regular;

	/** 检定成功（含大成功）后的跳转节点。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	FName SuccessNodeId;

	/** 检定失败后的跳转节点；策划要求失败也必须有内容，不允许为空。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	FName FailureNodeId;

	/** 前置线索；玩家未持有该线索时选项置灰。None 表示无条件。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	FName RequiredClueId;

	/** 前置剧情标记位。None 表示无条件。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	FName RequiredFlag;

	/** 该选项消耗的步数类型，用于归类与 HUD 提示。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	EILActionType StepCostType = EILActionType::Ask;

	/** 是否消耗步数（仅限步段实际扣减）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	bool bConsumeStep = true;

	/** 是否只能选择一次（同一节点已选过则置灰），防止重复刷检定拿技能点。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	bool bOnceOnly = false;
};

/**
 * 对话节点表行（DataTable 行名即 NodeId）。
 * 对话分支的最小单元：进入节点即应用副作用（必得线索/SAN/HP/标记/技能点），
 * 再由线性 NextNodeId 或选项列表决定去向；分支为「收敛式」——各选项最终汇合回主对话流。
 */
USTRUCT(BlueprintType)
struct FILDialogueNode : public FTableRowBase
{
	GENERATED_BODY()

	/** 说话人 ID："Player" 为主角，"Narrator" 为旁白，其余与队友/NPC 映射表对应。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName SpeakerId;

	/** 立绘资产键，对应 Content/UI/Portrait 的映射表；None 表示无立绘。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName PortraitId;

	/** 正文（玩家可见，走本地化，支持换行）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Text;

	/** 无选项时的线性下一节点；None 表示对话结束。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName NextNodeId;

	/** 选项列表；为空表示线性节点。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FILDialogueChoice> Choices;

	/** 进入节点即必得的关键线索（保证主线推进，与检定无关）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FName> GrantClues;

	/** 进入节点时的 SAN 变化，负数为损失（目睹真相/读禁书等）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 SanDelta = 0;

	/** 进入节点时的 HP 变化，负数为伤害。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 HpDelta = 0;

	/** 进入节点时设置的剧情标记位。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName SetFlag;

	/** 进入节点时授予的技能点（发现真相/支线奖励）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 GrantSkillPoints = 0;

	/** 是否为检查点（进入时自动存档）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bIsCheckpoint = false;

	/** 所属场景 ID，用于日志与线索归属校验。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName SceneId;
};

/**
 * 线索表行（DataTable 行名即 ClueId，CL-01 ... CL-30）。
 * 静态定义：分级、文案、证据图与结局权重；运行时收集状态存于 FILInvestigationState。
 */
USTRUCT(BlueprintType)
struct FILClueRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 线索名称（如「失踪者名单」）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
	FText DisplayName;

	/** 线索描述（证据图旁的说明文字）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
	FText Description;

	/** 关键线索（必得、保证推进）为 true；可选线索（检定获得、影响结局质量）为 false。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
	bool bIsKeyClue = false;

	/** 所属场景 ID（S1-1 等）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
	FName SceneId;

	/** 来源节点 ID，便于排查「这条线索为什么拿不到」。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
	FName SourceNodeId;

	/** 结局质量权重，数值越大对好结局贡献越高（结算/结局判定使用）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
	int32 EndingWeight = 1;

	/** 证据图资产键（2D 可放大展示图）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
	FName EvidenceImageId;
};

/**
 * 场景表行（DataTable 行名即 SceneId，S1-1 ... S3-2）。
 * 关卡装配配置：限步规则、入口对话与关卡资产引用；GameMode 进入场景时按此行初始化。
 */
USTRUCT(BlueprintType)
struct FILSceneRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 场景名称（如「灰港码头」）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
	FText DisplayName;

	/** 所属章节 ID（Chapter1 ... Chapter3）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
	FName ChapterId;

	/** 是否限步探索段（决定 HUD 是否显示剩余步数）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
	bool bLimitedSteps = false;

	/** 步数池覆盖值；0 表示用 FILAttributes::GetStepBudget 按玩家 INT 计算。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
	int32 StepBudgetOverride = 0;

	/** 场景入口对话节点（进入场景自动启动）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
	FName EntryDialogueNodeId;

	/** 关卡资产软引用（避免加载表时拉起全部关卡）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
	TSoftObjectPtr<UWorld> LevelAsset;

	/** 本章/本场景线索总数，用于信息完整度统计。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
	int32 TotalClueCount = 0;
};

/**
 * 检定配置表行（DataTable 行名即 CheckId）。
 * 集中管理检定点参数（技能、难度、失败惩罚、奖励），便于数值统一调优；
 * 对话选项自带的 Skill/Difficulty 可视为内联检定，此表用于独立事件检定与覆盖场景。
 */
USTRUCT(BlueprintType)
struct FILCheckConfigRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 检定使用的技能。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	EILSkill Skill = EILSkill::Investigation;

	/** 检定难度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	EILDifficulty Difficulty = EILDifficulty::Regular;

	/** 技能值覆盖。0 表示使用玩家当前技能值（用于固定难度的剧情检定/NPC 检定）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	int32 SkillValueOverride = 0;

	/** 失败时的 SAN 惩罚。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	int32 FailSanityPenalty = 0;

	/** 成功时授予的技能点。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	int32 SuccessSkillPoints = 1;

	/** 是否允许重试（策划要求失败推进剧情，默认不允许，失败即分叉）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bAllowRetry = false;
};
