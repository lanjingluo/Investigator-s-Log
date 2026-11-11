#include "Runtime/ILGameInstance.h"

#include "Data/ILGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/ILGameModeBase.h"
#include "Save/ILSaveGame.h"
#include "Subsystems/ILDialogueRunner.h"
#include "Subsystems/ILGameSessionSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogInvestigatorGame, Log, All);

void UILGameInstance::Init()
{
	Super::Init();
}

void UILGameInstance::ResetRun()
{
	// 清空周目数据并按属性回满；技能池/标记位/时间线随新模型一并归零
	PlayerModel = FILPlayerModel();
	PlayerModel.InitializeFromAttributes();
	if (const UILGameSettings* Settings = GetDefault<UILGameSettings>())
	{
		PlayerModel.SkillPoints = Settings->InitialSkillPoints;
	}
	CurrentCheckpointNodeId = NAME_None;
}

UILGameSessionSubsystem* UILGameInstance::GetSession() const
{
	// GameInstance 的同名子系统在 Init 阶段已创建，正常恒非空
	return GetSubsystem<UILGameSessionSubsystem>();
}

void UILGameInstance::LoadScene(FName SceneId)
{
	// 先记录目标场景，保证状态一致；关卡软引用跳转在步骤 6 接场景表后完善
	PlayerModel.CurrentSceneId = SceneId;
	UE_LOG(LogInvestigatorGame, Log, TEXT("[IL] LoadScene: %s"), *SceneId.ToString());
}

bool UILGameInstance::HasSaveRun() const
{
	return UGameplayStatics::DoesSaveGameExist(TEXT("ILSaveSlot"), 0);
}

bool UILGameInstance::SaveRun()
{
	UILSaveGame* SaveObj = Cast<UILSaveGame>(UGameplayStatics::CreateSaveGameObject(UILSaveGame::StaticClass()));
	if (SaveObj == nullptr)
	{
		return false;
	}

	// 模型与检查点直接快照；检定种子与一次性选项存在本关运行器里，经 GameMode 取出
	SaveObj->SaveVersion = UILSaveGame::CurrentSaveVersion;
	SaveObj->PlayerModel = PlayerModel;
	SaveObj->CurrentCheckpointNodeId = CurrentCheckpointNodeId;
	if (const UWorld* World = GetWorld())
	{
		if (const AILGameModeBase* GameMode = Cast<AILGameModeBase>(World->GetAuthGameMode()))
		{
			if (const UILDialogueRunner* Runner = GameMode->GetDialogueRunner())
			{
				SaveObj->RandomSeed = Runner->GetRollSeed();
				SaveObj->UsedChoiceIds = Runner->GetUsedChoiceIds();
			}
		}
	}

	return UGameplayStatics::SaveGameToSlot(SaveObj, TEXT("ILSaveSlot"), 0);
}

bool UILGameInstance::LoadRun()
{
	UILSaveGame* SaveObj = Cast<UILSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("ILSaveSlot"), 0));
	if (SaveObj == nullptr || SaveObj->SaveVersion != UILSaveGame::CurrentSaveVersion)
	{
		// 无档或版本不符：直接判失败，由调用方决定回落新周目
		return false;
	}

	PlayerModel = SaveObj->PlayerModel;
	CurrentCheckpointNodeId = SaveObj->CurrentCheckpointNodeId;

	// 运行器状态先暂存：Runner 每关新建，由 GameMode 装配时注入
	PendingUsedChoiceIds = SaveObj->UsedChoiceIds;
	PendingRollSeed = SaveObj->RandomSeed;
	bHasPendingRunnerState = true;

	// 恢复后立即走既有委托链刷新 HUD（HP/SAN/步数）
	if (UILGameSessionSubsystem* Session = GetSession())
	{
		Session->BroadcastStats();
	}
	return true;
}
