#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ILPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UILInteractionComponent;

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

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** 交互键触发（转发给交互检测组件）。 */
	void OnInteractPressed();

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
};
