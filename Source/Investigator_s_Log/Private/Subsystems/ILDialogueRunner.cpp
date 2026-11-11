#include "Subsystems/ILDialogueRunner.h"

#include "Core/ILCheckSystem.h"
#include "Data/ILGameSettings.h"
#include "Engine/DataTable.h"
#include "Runtime/ILGameInstance.h"
#include "Subsystems/ILGameSessionSubsystem.h"

void UILDialogueRunner::Initialize(UDataTable* InDialogueTable, UDataTable* InClueTable, UILGameSessionSubsystem* InSession)
{
	DialogueTable = InDialogueTable;
	ClueTable = InClueTable;
	Session = InSession;
	CurrentNodeId = NAME_None;
	bWaitingForRoll = false;
	UsedChoiceIds.Reset();
	// 独立随机化种子；RestoreState 注入存档种子后检定序列可复现
	RollStream.Initialize(FMath::Rand());
}

void UILDialogueRunner::RestoreState(const TSet<FName>& InUsedChoiceIds, int32 InSeed)
{
	// 读档恢复：一次性选项与检定种子来自存档快照（GameMode 装配本关运行器后调用）
	UsedChoiceIds = InUsedChoiceIds;
	RollStream.Initialize(InSeed);
}

void UILDialogueRunner::StartAt(FName NodeId)
{
	EnterNode(NodeId);
}

void UILDialogueRunner::Advance()
{
	if (bWaitingForRoll)
	{
		return;
	}
	FILDialogueNode Node;
	if (!GetCurrentNode(Node))
	{
		return;
	}
	// 带选项的节点必须显式选择，线性推进仅对无选项节点生效
	if (Node.Choices.Num() > 0)
	{
		return;
	}
	if (!Node.NextNodeId.IsNone())
	{
		EnterNode(Node.NextNodeId);
	}
}

bool UILDialogueRunner::GetCurrentNode(FILDialogueNode& OutNode) const
{
	if (DialogueTable == nullptr || CurrentNodeId.IsNone())
	{
		return false;
	}
	const FILDialogueNode* FoundNode = DialogueTable->FindRow<FILDialogueNode>(CurrentNodeId, TEXT("GetCurrentNode"), true);
	if (FoundNode == nullptr)
	{
		return false;
	}
	OutNode = *FoundNode;
	return true;
}

TArray<FILDialogueChoice> UILDialogueRunner::GetAvailableChoices() const
{
	FILDialogueNode Node;
	return GetCurrentNode(Node) ? Node.Choices : TArray<FILDialogueChoice>();
}

bool UILDialogueRunner::IsChoiceEnabled(const FILDialogueChoice& Choice) const
{
	if (Session == nullptr)
	{
		return false;
	}
	const FILPlayerModel& Model = Session->GetModel();
	// 前置线索未持有 → 置灰（信息缺失下的抉择设计：灰选项可见但不可点）
	if (!Choice.RequiredClueId.IsNone() && !Model.Investigation.HasClue(Choice.RequiredClueId))
	{
		return false;
	}
	// 前置剧情标记不满足 → 置灰
	if (!Choice.RequiredFlag.IsNone() && !Model.Flags.Contains(Choice.RequiredFlag))
	{
		return false;
	}
	// 一次性选项已使用 → 置灰，防止重复刷检定拿技能点
	if (Choice.bOnceOnly && !Choice.ChoiceId.IsNone() && UsedChoiceIds.Contains(Choice.ChoiceId))
	{
		return false;
	}
	return true;
}

float UILDialogueRunner::GetChoiceSuccessChance(const FILDialogueChoice& Choice) const
{
	if (Session == nullptr)
	{
		return 0.0f;
	}
	const UILGameSettings* Settings = GetDefault<UILGameSettings>();
	return UILCheckSystem::GetSuccessChance(
		Session->GetPlayerSkillValue(Choice.Skill),
		Choice.Difficulty,
		Settings ? Settings->bUseCriticalSuccess : true,
		Settings ? Settings->CriticalFailureMin : 96);
}

void UILDialogueRunner::SelectChoice(int32 ChoiceIndex)
{
	if (bWaitingForRoll || Session == nullptr)
	{
		return;
	}
	const TArray<FILDialogueChoice> Choices = GetAvailableChoices();
	if (!Choices.IsValidIndex(ChoiceIndex))
	{
		return;
	}
	const FILDialogueChoice& Choice = Choices[ChoiceIndex];
	if (!IsChoiceEnabled(Choice))
	{
		return;
	}

	// 步数门槛：限步段不足时直接拒绝（UI 侧可提示「步数已耗尽」）
	if (Choice.bConsumeStep && !Session->TryConsumeStep(Choice.StepCostType, 1))
	{
		return;
	}
	if (Choice.bOnceOnly && !Choice.ChoiceId.IsNone())
	{
		UsedChoiceIds.Add(Choice.ChoiceId);
	}

	// 无检定选项：线性进入结果节点（无检定语义下复用 SuccessNodeId 承载跳转）
	if (!Choice.bRequiresCheck)
	{
		EnterNode(Choice.SuccessNodeId);
		return;
	}

	// 需要检定：立即掷骰并缓存快照，广播请求后等待 UI 动画回调 ResolveRoll
	const UILGameSettings* Settings = GetDefault<UILGameSettings>();
	PendingRoll.Skill = Choice.Skill;
	UILCheckSystem::RollCheck(
		Session->GetPlayerSkillValue(Choice.Skill),
		Choice.Difficulty,
		RollStream,
		PendingRoll,
		Settings ? Settings->bUseCriticalSuccess : true,
		Settings ? Settings->CriticalSuccessMax : 5,
		Settings ? Settings->CriticalFailureMin : 96);
	PendingSuccessNodeId = Choice.SuccessNodeId;
	PendingFailureNodeId = Choice.FailureNodeId;
	bWaitingForRoll = true;
	Session->OnDiceRollRequested.Broadcast(PendingRoll);
}

void UILDialogueRunner::ResolveRoll()
{
	if (!bWaitingForRoll)
	{
		return;
	}
	bWaitingForRoll = false;
	ApplyRollOutcome(PendingRoll.Result);
	Session->OnDiceRollResolved.Broadcast();
}

void UILDialogueRunner::ApplyRollOutcome(EILRollResult Result)
{
	const bool bSuccess = (Result == EILRollResult::Success || Result == EILRollResult::CriticalSuccess);
	// 大成功/大失败的特殊剧情分支待策划补表后接入，当前与普通成败走相同节点
	EnterNode(bSuccess ? PendingSuccessNodeId : PendingFailureNodeId);
}

void UILDialogueRunner::EnterNode(FName NodeId)
{
	CurrentNodeId = NodeId;
	if (Session == nullptr)
	{
		return;
	}

	const FILDialogueNode* Node = (DialogueTable != nullptr && !NodeId.IsNone())
		? DialogueTable->FindRow<FILDialogueNode>(NodeId, TEXT("EnterNode"), true)
		: nullptr;
	if (Node == nullptr)
	{
		// 空节点 = 对话结束或数据缺失：仅广播让 UI 收起对话，不视为错误
		Session->OnDialogueNodeChanged.Broadcast(NodeId);
		return;
	}

	// 进入节点副作用按固定顺序应用：必得线索（关键线索保证主线推进）→ SAN → HP → 标记 → 技能点
	for (const FName ClueId : Node->GrantClues)
	{
		Session->GrantClue(ClueId);
	}
	if (Node->SanDelta != 0)
	{
		Session->ApplySANDelta(Node->SanDelta);
	}
	if (Node->HpDelta != 0)
	{
		Session->ApplyHPDelta(Node->HpDelta);
	}
	if (!Node->SetFlag.IsNone())
	{
		Session->SetFlag(Node->SetFlag);
	}
	if (Node->GrantSkillPoints != 0)
	{
		Session->AddSkillPoints(Node->GrantSkillPoints);
	}

	// 检查点节点：记录到 GameInstance，读档后可从此恢复对话
	if (Node->bIsCheckpoint)
	{
		if (UGameInstance* GameInst = Session->GetGameInstance())
		{
			if (UILGameInstance* ILGameInst = Cast<UILGameInstance>(GameInst))
			{
				ILGameInst->CurrentCheckpointNodeId = NodeId;
			}
		}
	}

	Session->OnDialogueNodeChanged.Broadcast(NodeId);
}
