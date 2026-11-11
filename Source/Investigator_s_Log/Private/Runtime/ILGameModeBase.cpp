#include "Runtime/ILGameModeBase.h"

#include "Engine/DataTable.h"
#include "Runtime/ILGameInstance.h"
#include "Subsystems/ILDialogueRunner.h"
#include "Subsystems/ILGameSessionSubsystem.h"

AILGameModeBase::AILGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AILGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 会话子系统：周目数据与委托广播的中枢
	UILGameInstance* ILGameInst = Cast<UILGameInstance>(GetGameInstance());
	Session = ILGameInst ? ILGameInst->GetSession() : nullptr;
	if (Session == nullptr)
	{
		return;
	}

	// 按场景表初始化步数段（限步开关、步数池、线索计数）
	if (SceneTable != nullptr && !SceneId.IsNone())
	{
		if (const FILSceneRow* SceneRow = SceneTable->FindRow<FILSceneRow>(SceneId, TEXT("GameModeBeginPlay")))
		{
			Session->BeginScene(*SceneRow, SceneId);
		}
	}

	// 每关新建对话运行器并绑定数据表（状态不跨关携带，降低脏状态风险）
	if (DialogueRunner == nullptr)
	{
		DialogueRunner = NewObject<UILDialogueRunner>(this);
		DialogueRunner->Initialize(DialogueTable, ClueTable, Session);

		// 读档恢复：把一次性选项与检定种子注入本关新运行器（一次性消费）
		if (ILGameInst != nullptr && ILGameInst->HasPendingRunnerState())
		{
			DialogueRunner->RestoreState(ILGameInst->GetPendingUsedChoiceIds(), ILGameInst->GetPendingRollSeed());
			ILGameInst->ClearPendingRunnerState();
		}
	}
}

void AILGameModeBase::StartDialogue(FName StartNodeId)
{
	if (DialogueRunner != nullptr)
	{
		DialogueRunner->StartAt(StartNodeId);
	}
}

bool AILGameModeBase::ResumeFromCheckpoint()
{
	// 读档后从 GameInstance 检查点恢复；对话未经过任何检查点时静默提示
	UILGameInstance* ILGameInst = Cast<UILGameInstance>(GetGameInstance());
	const FName CheckpointId = ILGameInst ? ILGameInst->CurrentCheckpointNodeId : NAME_None;
	if (CheckpointId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[IL] ResumeFromCheckpoint：无检查点（对话尚未经过 bIsCheckpoint 节点），忽略"));
		return false;
	}
	if (DialogueRunner == nullptr)
	{
		return false;
	}
	DialogueRunner->StartAt(CheckpointId);
	return true;
}

void AILGameModeBase::FinishRun(FName EndingId)
{
	if (Session != nullptr)
	{
		Session->FinishRun(EndingId);
	}
}
