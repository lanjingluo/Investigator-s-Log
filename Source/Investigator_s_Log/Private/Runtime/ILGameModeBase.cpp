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
	}
}

void AILGameModeBase::StartDialogue(FName StartNodeId)
{
	// [IL诊断] 对话链路第1站：确认蓝图调用了本函数且表已挂载
	UE_LOG(LogTemp, Warning, TEXT("[IL诊断] StartDialogue被调用：NodeId=%s，DialogueTable=%s，Runner=%s"),
		*StartNodeId.ToString(),
		DialogueTable ? *DialogueTable->GetName() : TEXT("【空！GameMode未挂对话表】"),
		DialogueRunner ? TEXT("有效") : TEXT("【空！BeginPlay未创建】"));
	if (DialogueRunner != nullptr)
	{
		DialogueRunner->StartAt(StartNodeId);
	}
}

void AILGameModeBase::FinishRun(FName EndingId)
{
	if (Session != nullptr)
	{
		Session->FinishRun(EndingId);
	}
}
