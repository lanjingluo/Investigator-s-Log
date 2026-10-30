#include "Subsystems/ILGameSessionSubsystem.h"

#include "Data/ILGameSettings.h"
#include "Runtime/ILGameInstance.h"

void UILGameSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 缓存全局设置（Config 类单例，读取零开销）并随机化掷骰种子
	CachedSettings = GetMutableDefault<UILGameSettings>();
	RollStream = FRandomStream(FMath::Rand());
}

const FILPlayerModel& UILGameSessionSubsystem::GetModel() const
{
	// 正式运行流程 GameInstance 恒为 UILGameInstance；
	// 类型异常（如默认 GameInstance 的编辑器场景）时回退静态空模型，避免解引用崩溃
	const UILGameInstance* ILGameInst = Cast<UILGameInstance>(GetGameInstance());
	static const FILPlayerModel EmptyModel;
	return ILGameInst ? ILGameInst->GetModel() : EmptyModel;
}

int32 UILGameSessionSubsystem::GetPlayerSkillValue(EILSkill Skill) const
{
	return GetModel().Skills.GetSkill(Skill);
}

FRandomStream& UILGameSessionSubsystem::GetRollStream()
{
	return RollStream;
}

FILPlayerModel& UILGameSessionSubsystem::GetModelMutable()
{
	// 与 GetModel 同源；Cast 失败时使用静态可写占位（仅在异常流程出现）
	UILGameInstance* ILGameInst = Cast<UILGameInstance>(GetGameInstance());
	static FILPlayerModel FallbackModel;
	return ILGameInst ? ILGameInst->GetModelMutable() : FallbackModel;
}

void UILGameSessionSubsystem::ApplyHPDelta(int32 Delta)
{
	FILPlayerModel& Model = GetModelMutable();
	const int32 OldHP = Model.CurrentHP;
	Model.ApplyHP(Delta);
	// 隐藏机制：HP 归零瞬间才记一次昏迷账（苏醒时间拖延 + 终章难度 +1），全程无 UI 提示
	if (Model.CurrentHP <= 0 && OldHP > 0)
	{
		const int32 WastedHours = CachedSettings ? CachedSettings->WastedHoursPerFaint : 2;
		Model.Time.ApplyFaint(WastedHours);
	}
	BroadcastStats();
}

void UILGameSessionSubsystem::ApplySANDelta(int32 Delta)
{
	// SAN 归零仅做数值钳制；癫狂演出由剧情层按 bIsBroken 语义处理
	GetModelMutable().ApplySAN(Delta);
	BroadcastStats();
}

void UILGameSessionSubsystem::GrantClue(FName ClueId)
{
	FILPlayerModel& Model = GetModelMutable();
	Model.Investigation.AddClue(ClueId);
	// 附带信息完整度，UI 可直接显示「线索收集 3/10」
	OnClueCollected.Broadcast(ClueId, Model.Investigation.GetCompletionRate(CurrentTotalClueCount));
}

bool UILGameSessionSubsystem::TryConsumeStep(EILActionType ActionType, int32 Cost)
{
	// ActionType 当前仅用于日志归类；限步段不足时返回 false 由调用方阻止交互
	const bool bSpent = GetModelMutable().Time.TrySpendStep(Cost);
	if (bSpent)
	{
		BroadcastStats();
	}
	return bSpent;
}

void UILGameSessionSubsystem::BeginScene(const FILSceneRow& SceneRow, FName SceneId)
{
	FILPlayerModel& Model = GetModelMutable();
	CurrentTotalClueCount = SceneRow.TotalClueCount;

	// 步数池：场景表覆盖值优先，否则按属性公式 + 全局参数计算；自由段传 0
	int32 Budget = SceneRow.StepBudgetOverride;
	if (Budget <= 0)
	{
		Budget = Model.Attributes.GetStepBudget(
			CachedSettings ? CachedSettings->StepBudgetBase : 15,
			CachedSettings ? CachedSettings->StepBudgetIntDivisor : 1.5f,
			CachedSettings ? CachedSettings->StepBudgetMin : 8,
			CachedSettings ? CachedSettings->StepBudgetMax : 25);
	}
	Model.Time.BeginScene(SceneRow.bLimitedSteps ? Budget : 0);
	Model.CurrentSceneId = SceneId;
	BroadcastStats();
}

void UILGameSessionSubsystem::SetFlag(FName Flag)
{
	GetModelMutable().Flags.Add(Flag);
}

bool UILGameSessionSubsystem::HasFlag(FName Flag) const
{
	return GetModel().Flags.Contains(Flag);
}

void UILGameSessionSubsystem::AddSkillPoints(int32 Points)
{
	GetModelMutable().SkillPoints += Points;
}

void UILGameSessionSubsystem::FinishRun(FName EndingId)
{
	// 结算界面（唯一允许点破昏迷隐藏机制的界面）将读取 Time.FaintCount / WastedHours
	OnRunFinished.Broadcast(EndingId);
}

void UILGameSessionSubsystem::BroadcastStats()
{
	const FILPlayerModel& Model = GetModel();
	OnStatChanged.Broadcast(Model.CurrentHP, Model.CurrentSAN, Model.Time.GetRemainingSteps());
}
