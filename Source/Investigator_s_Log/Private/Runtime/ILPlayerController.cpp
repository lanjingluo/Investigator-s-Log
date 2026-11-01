#include "Runtime/ILPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputMappingContext.h"
#include "Runtime/ILInteractionComponent.h"

AILPlayerController::AILPlayerController()
{
	// 交互检测组件挂在控制器上，与 Pawn 解耦：任何 Pawn 都能复用同一套交互逻辑
	InteractionComponent = CreateDefaultSubobject<UILInteractionComponent>(TEXT("InteractionComponent"));
}

void AILPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 俯视角点选交互依赖常显鼠标，并默认进入探索上下文
	SetShowMouseCursor(true);

	// UE5.8 兼容修复：引擎可能把 PlayerInput 创建为旧版 UPlayerInput（不路由 Enhanced 按键），
	// 强制替换为 EnhancedPlayerInput，否则 WASD/E 全部静默失效
	if (PlayerInput == nullptr || !PlayerInput->IsA<UEnhancedPlayerInput>())
	{
		PlayerInput = NewObject<UEnhancedPlayerInput>(this);
	}
	SwitchInputContext(FName(TEXT("Exploration")));
}

void AILPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Enhanced Input 绑定：交互键按下即触发
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InteractAction != nullptr)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AILPlayerController::OnInteractPressed);
		}
	}
}

void AILPlayerController::OnInteractPressed()
{
	if (InteractionComponent != nullptr)
	{
		InteractionComponent->TriggerInteract();
	}
}

void AILPlayerController::SwitchInputContext(FName ContextName)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (InputSubsystem == nullptr)
	{
		return;
	}

	// 先清空全部上下文再按需添加，保证三态互斥且优先级恒定
	if (ExplorationContext != nullptr)
	{
		InputSubsystem->RemoveMappingContext(ExplorationContext);
	}
	if (DialogueContext != nullptr)
	{
		InputSubsystem->RemoveMappingContext(DialogueContext);
	}
	if (MenuContext != nullptr)
	{
		InputSubsystem->RemoveMappingContext(MenuContext);
	}

	UInputMappingContext* TargetContext = ExplorationContext;
	if (ContextName == TEXT("Dialogue"))
	{
		TargetContext = DialogueContext;
	}
	else if (ContextName == TEXT("Menu"))
	{
		TargetContext = MenuContext;
	}
	if (TargetContext != nullptr)
	{
		InputSubsystem->AddMappingContext(TargetContext, 0);
	}
}

void AILPlayerController::SetUIInputMode(bool bUIOnly)
{
	if (bUIOnly)
	{
		// 仅 UI：弹窗/对话期间暂停游戏侧输入
		FInputModeUIOnly Mode;
		SetInputMode(Mode);
	}
	else
	{
		// Game+UI：探索态同时接收点击与 UI 悬停
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
	}
	SetShowMouseCursor(true);
}
