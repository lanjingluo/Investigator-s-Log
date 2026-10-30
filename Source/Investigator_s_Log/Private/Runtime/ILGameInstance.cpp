#include "Runtime/ILGameInstance.h"

#include "Data/ILGameSettings.h"
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
