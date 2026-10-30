#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ILInteractionComponent.generated.h"

class AILPlayerController;

/**
 * 交互检测组件（挂在 PlayerController 上）：
 * 每帧从鼠标屏幕位置反投影为世界射线做可视性检测，缓存命中的可交互目标，
 * 供 HUD 显示「[E] 提示」并响应交互键。目标用弱引用持有避免 GC 泄漏。
 */
UCLASS(ClassGroup = (Investigator), meta = (BlueprintSpawnableComponent))
class UILInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UILInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 当前指向的可交互目标（可能为空）。 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	/** 当前目标的提示文本；无目标时返回空文本。 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetCurrentPrompt() const;

	/** 触发当前目标的交互（交互键回调入口）。 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TriggerInteract();

protected:
	/** 射线最大距离（俯视相机下需覆盖屏幕到地面全程）。 */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float TraceDistance = 50000.0f;

private:
	/** 当前命中的可交互 Actor（弱引用，防止持有导致的对象滞留）。 */
	TWeakObjectPtr<AActor> CurrentTarget;
};
