#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ILInteractable.generated.h"

class AILPlayerController;

/** 可交互物接口（UInterface 声明）。 */
UINTERFACE(MinimalAPI, Blueprintable)
class UILInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可交互物接口：任何 Actor 实现本接口即可被鼠标/E 键交互。
 * 实现方式：C++ 实现类重写 *_Implementation，蓝图实现类直接覆写同名函数。
 */
class INVESTIGATOR_S_LOG_API IILInteractable
{
	GENERATED_BODY()

public:
	/**
	 * 交互提示文本（如「[E] 查看船票」），供 HUD 悬停显示。
	 * @return 提示文本；无目标时由调用方处理为空
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractPrompt() const;

	/**
	 * 执行交互（对话启动 / 搜查 / 场景跳转等，由实现方决定）。
	 * @param Controller 发起交互的玩家控制器
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(AILPlayerController* Controller);
};
