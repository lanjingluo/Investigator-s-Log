#include "Runtime/ILInteractionComponent.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/ILInteractable.h"
#include "Runtime/ILPlayerController.h"

UILInteractionComponent::UILInteractionComponent()
{
	// 每帧检测鼠标指向，开销可忽略（单条射线）
	PrimaryComponentTick.bCanEverTick = true;
}

void UILInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	UGameViewportClient* Viewport = GEngine ? GEngine->GameViewport.Get() : nullptr;
	UWorld* World = GetWorld();
	if (PlayerController == nullptr || World == nullptr || Viewport == nullptr)
	{
		CurrentTarget = nullptr;
		return;
	}

	// 鼠标屏幕坐标 → 世界射线
	FVector2D MousePosition;
	if (!Viewport->GetMousePosition(MousePosition))
	{
		CurrentTarget = nullptr;
		return;
	}
	FVector WorldOrigin;
	FVector WorldDirection;
	if (!PlayerController->DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldOrigin, WorldDirection))
	{
		CurrentTarget = nullptr;
		return;
	}

	// 可视性通道射线检测，命中物必须实现 IILInteractable 才算有效目标
	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ILInteraction), false);
	const FVector TraceEnd = WorldOrigin + WorldDirection * TraceDistance;
	if (World->LineTraceSingleByChannel(HitResult, WorldOrigin, TraceEnd, ECC_Visibility, QueryParams))
	{
		AActor* HitActor = HitResult.GetActor();
		CurrentTarget = (HitActor != nullptr && HitActor->GetClass()->ImplementsInterface(UILInteractable::StaticClass())) ? HitActor : nullptr;
	}
	else
	{
		CurrentTarget = nullptr;
	}
}

FText UILInteractionComponent::GetCurrentPrompt() const
{
	AActor* Target = CurrentTarget.Get();
	return Target != nullptr ? IILInteractable::Execute_GetInteractPrompt(Target) : FText::GetEmpty();
}

void UILInteractionComponent::TriggerInteract()
{
	AActor* Target = CurrentTarget.Get();
	AILPlayerController* ILController = Cast<AILPlayerController>(GetOwner());
	if (Target != nullptr && ILController != nullptr)
	{
		// 通过生成的 Execute_ 静态帮助器调用，兼容蓝图实现
		IILInteractable::Execute_OnInteract(Target, ILController);
	}
}
