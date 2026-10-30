#include "Runtime/ILTopDownPawn.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"

AILTopDownPawn::AILTopDownPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 俯视相机直接挂根组件：抬高 900 并俯角 -60°，实现类博德之门俯视角
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 900.f));
	Camera->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
}

void AILTopDownPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AILTopDownPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 点选移动：逐帧朝目标推进，进入 10cm 阈值后停止（防末端抖动）
	if (bHasMoveTarget)
	{
		FVector ToTarget = MoveTargetLocation - GetActorLocation();
		ToTarget.Z = 0.f;
		if (ToTarget.SizeSquared() <= 100.f)
		{
			bHasMoveTarget = false;
			return;
		}
		AddMovementInput(ToTarget.GetSafeNormal());
	}
}

void AILTopDownPawn::Move(FVector2D Axis)
{
	AddMovementInput(FVector::RightVector, Axis.X);
	AddMovementInput(FVector::ForwardVector, Axis.Y);
}

bool AILTopDownPawn::MoveToScreenPosition(const FVector2D& ScreenPosition)
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController == nullptr)
	{
		return false;
	}

	FVector WorldOrigin;
	FVector WorldDirection;
	if (!PlayerController->DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldOrigin, WorldDirection))
	{
		return false;
	}

	// 与 Z=0 地面平面求交：T = -Origin.Z / Dir.Z；射线平行或背向地面均无效
	if (FMath::IsNearlyZero(WorldDirection.Z))
	{
		return false;
	}
	const float IntersectionT = -WorldOrigin.Z / WorldDirection.Z;
	if (IntersectionT <= 0.f)
	{
		return false;
	}

	MoveTargetLocation = WorldOrigin + WorldDirection * IntersectionT;
	bHasMoveTarget = true;
	return true;
}
