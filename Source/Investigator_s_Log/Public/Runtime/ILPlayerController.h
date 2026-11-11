#pragma once

#include "CoreMinimal.h"
#include "Core/ILTypes.h"
#include "GameFramework/PlayerController.h"
#include "ILPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UILDialogueRunner;
class UILDialogueWidget;
class UILDiceRollWidget;
class UILHUDWidget;
class UILInteractionComponent;
class UILResultWidget;

/**
 * 玩家控制器：负责输入上下文切换（探索/对话/菜单）、UI 输入模式与交互触发。
 * 不承载任何游戏规则判定（规则在 Core 层与会话子系统）。
 */
UCLASS()
class AILPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AILPlayerController();

	/**
	 * 切换输入映射上下文，三态互斥。
	 * @param ContextName "Exploration" / "Dialogue" / "Menu"
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SwitchInputContext(FName ContextName);

	/**
	 * 切换 UI 输入模式。
	 * @param bUIOnly true 为仅 UI（弹窗时暂停移动）；false 为 Game+UI 并行
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetUIInputMode(bool bUIOnly);

	/** HUD 界面类（蓝图内指定 WBP_HUD）。 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UILHUDWidget> HUDWidgetClass;

	/** 对话界面类（WBP_Dialogue）。 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UILDialogueWidget> DialogueWidgetClass;

	/** 骰子面板类（WBP_DiceRoll）。 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UILDiceRollWidget> DiceRollWidgetClass;

	/** 结算界面类（WBP_Result）。 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UILResultWidget> ResultWidgetClass;

protected:
	/** BeginPlay 后创建常驻 UI 并订阅会话委托（对话推进/骰子请求/结算）。 */
	void InitSessionUI();

	/** 对话节点推进（NodeId 为 None 表示对话结束，收起界面并切回探索态）。 */
	UFUNCTION()
	void HandleDialogueNodeChanged(FName NodeId);

	/** 骰子检定请求：弹出检定面板，倒计时结束后由面板回调 ResolveRoll。 */
	UFUNCTION()
	void HandleDiceRollRequested(const FILDiceRoll& Roll);

	/** 周目结束：显示结算界面（唯一点破隐藏机制处）。 */
	UFUNCTION()
	void HandleRunFinished(FName EndingId);
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** 交互键触发（转发给交互检测组件）。 */
	void OnInteractPressed();

	/** 调试：F5 保存周目（存档槽 ILSaveSlot），屏幕消息反馈结果。 */
	UFUNCTION()
	void DebugSaveRun();

	/** 调试：F9 读取周目并刷新 HUD，屏幕消息反馈结果。 */
	UFUNCTION()
	void DebugLoadRun();

	/** 探索态输入映射上下文。 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> ExplorationContext;

	/** 对话态输入映射上下文。 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DialogueContext;

	/** 菜单态输入映射上下文。 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MenuContext;

	/** 交互输入动作（E 键）。 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	/** 交互检测组件（控制器持有，与 Pawn 解耦）。 */
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TObjectPtr<UILInteractionComponent> InteractionComponent;

	/** 常驻 HUD 实例。 */
	UPROPERTY()
	TObjectPtr<UILHUDWidget> HUDWidget;

	/** 对话界面实例。 */
	UPROPERTY()
	TObjectPtr<UILDialogueWidget> DialogueWidget;

	/** 骰子面板实例。 */
	UPROPERTY()
	TObjectPtr<UILDiceRollWidget> DiceRollWidget;

	/** 结算界面实例（首次结算时懒创建）。 */
	UPROPERTY()
	TObjectPtr<UILResultWidget> ResultWidget;

	/** 会话委托是否已订阅（防重复绑定）。 */
	bool bSessionDelegatesBound = false;
};
