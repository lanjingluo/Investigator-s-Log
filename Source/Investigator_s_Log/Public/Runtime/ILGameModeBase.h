#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ILGameModeBase.generated.h"

class UDataTable;
class UILDialogueRunner;
class UILGameSessionSubsystem;

/**
 * 关卡级 GameMode：进入场景时按场景表初始化步数段，并装配本关对话运行器。
 * 周目数据仍存于 GameInstance，本类只做「每关装配」。
 */
UCLASS()
class AILGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AILGameModeBase();

protected:
	virtual void BeginPlay() override;

public:
	/** 当前场景 ID（关卡蓝图或 GameInstance 设置）。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scene")
	FName SceneId;

	/** 场景表资产（行结构 FILSceneRow）。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scene")
	TObjectPtr<UDataTable> SceneTable;

	/** 对话表资产（行结构 FILDialogueNode）。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<UDataTable> DialogueTable;

	/** 线索表资产（行结构 FILClueRow，供运行器扩展校验）。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<UDataTable> ClueTable;

	/** 开始一段对话（转发给对话运行器）。 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(FName StartNodeId);

	/**
	 * 从最近检查点恢复对话（读档后调用）；无检查点时不做任何事并输出日志。
	 * @return 是否成功从检查点开始
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool ResumeFromCheckpoint();

	/** 结束周目进入结算。 */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void FinishRun(FName EndingId);

	/**
	 * 取本关对话运行器（PC 转发对话 UI 事件用）。
	 * @return 运行器指针；关卡未配置对话表时为 nullptr
	 */
	UILDialogueRunner* GetDialogueRunner() const { return DialogueRunner.Get(); }

protected:
	/** 会话子系统缓存（BeginPlay 时获取）。 */
	UPROPERTY()
	TObjectPtr<UILGameSessionSubsystem> Session;

	/** 本关对话运行器（每关新建，状态不跨关携带）。 */
	UPROPERTY()
	TObjectPtr<UILDialogueRunner> DialogueRunner;
};
