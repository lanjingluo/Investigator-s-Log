#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ILTopDownPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;

/**
 * 俯视角观察者 Pawn：固定俯视相机 + 平面移动（WASD / 鼠标点选）。
 * 有意不用 Character（无跳跃与重力需求），FloatingPawnMovement 足够且省去胶囊体复杂度；
 * 相机以固定偏移挂载（俯视 -60°、抬高 900），避免对 SpringArm 头文件的引擎版本路径依赖。
 */
UCLASS()
class AILTopDownPawn : public APawn
{
	GENERATED_BODY()

public:
	AILTopDownPawn();

	/**
	 * 平面移动输入（WASD 键盘方案）。
	 * @param Axis X 为左右（世界右向量），Y 为前后（世界前向量）
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Move(FVector2D Axis);

	/**
	 * 点选移动：把屏幕坐标反投影为世界射线，与地面 Z=0 平面求交作为移动目标。
	 * @param ScreenPosition 鼠标屏幕坐标
	 * @return 成功设置移动目标返回 true；反投影失败或射线不朝地面返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool MoveToScreenPosition(const FVector2D& ScreenPosition);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** 俯视相机（固定偏移：抬高 900、俯角 -60°）。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	/** 平面移动组件（无重力）。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

private:
	/** 点选移动目标点（Z 恒为 0 平面上的交点）。 */
	FVector MoveTargetLocation = FVector::ZeroVector;

	/** 是否存在有效点选目标。 */
	bool bHasMoveTarget = false;
};
