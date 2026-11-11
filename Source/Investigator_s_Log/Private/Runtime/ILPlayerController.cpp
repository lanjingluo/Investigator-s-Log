#include "Runtime/ILPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Data/ILDataTypes.h"
#include "Engine/Engine.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Runtime/ILGameInstance.h"
#include "Runtime/ILGameModeBase.h"
#include "Runtime/ILInteractionComponent.h"
#include "Subsystems/ILDialogueRunner.h"
#include "Subsystems/ILGameSessionSubsystem.h"
#include "UI/ILDialogueWidget.h"
#include "UI/ILDiceRollWidget.h"
#include "UI/ILHUDWidget.h"
#include "UI/ILResultWidget.h"

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

	InitSessionUI();
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

	// 调试键：F5 存档 / F9 读档（旧版 BindKey 与 EnhancedInput 共存，无需 IMC 映射）
	InputComponent->BindKey(EKeys::F5, IE_Pressed, this, &AILPlayerController::DebugSaveRun);
	InputComponent->BindKey(EKeys::F9, IE_Pressed, this, &AILPlayerController::DebugLoadRun);
}

void AILPlayerController::OnInteractPressed()
{
	if (InteractionComponent != nullptr)
	{
		InteractionComponent->TriggerInteract();
	}
}

void AILPlayerController::DebugSaveRun()
{
	// 调试存档：结果直接打到屏幕，省去翻日志
	if (UILGameInstance* GameInst = Cast<UILGameInstance>(GetGameInstance()))
	{
		const bool bSaved = GameInst->SaveRun();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, bSaved ? FColor::Green : FColor::Red,
			bSaved ? TEXT("[IL] F5 已保存周目") : TEXT("[IL] F5 保存失败（无 GameInstance 或写盘失败）"));
	}
}

void AILPlayerController::DebugLoadRun()
{
	// 调试读档：LoadRun 内部已恢复模型并广播刷新 HUD
	if (UILGameInstance* GameInst = Cast<UILGameInstance>(GetGameInstance()))
	{
		const bool bLoaded = GameInst->LoadRun();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, bLoaded ? FColor::Green : FColor::Red,
			bLoaded ? TEXT("[IL] F9 已读取周目，HUD 应已刷新") : TEXT("[IL] F9 无存档或版本不符"));
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
		// 对话/结算态：改用 GameAndUI——Enhanced IMC 已切走（WASD 无效），
		// 但保留游戏侧旧版键绑定（F5/F9 存读档调试键在 UIOnly 下会被吞掉）
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
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

void AILPlayerController::InitSessionUI()
{
	UILGameInstance* InvestigatorGI = Cast<UILGameInstance>(GetGameInstance());
	UILGameSessionSubsystem* Session = InvestigatorGI ? InvestigatorGI->GetSession() : nullptr;
	if (Session == nullptr || bSessionDelegatesBound)
	{
		return;
	}

	// 会话委托是 UI 的唯一驱动源：PC 只做中转，把事件翻译成各界面的显隐与刷新
	Session->OnDialogueNodeChanged.AddDynamic(this, &AILPlayerController::HandleDialogueNodeChanged);
	Session->OnDiceRollRequested.AddDynamic(this, &AILPlayerController::HandleDiceRollRequested);
	Session->OnRunFinished.AddDynamic(this, &AILPlayerController::HandleRunFinished);
	Session->OnDiceRollResolved.AddDynamic(this, &AILPlayerController::HandleDiceRollResolved);
	bSessionDelegatesBound = true;

	// 常驻 HUD 立即上屏；对话与骰子面板创建后先收起，由事件驱动显示
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UILHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	if (DialogueWidgetClass)
	{
		DialogueWidget = CreateWidget<UILDialogueWidget>(this, DialogueWidgetClass);
		if (DialogueWidget)
		{
			DialogueWidget->AddToViewport();
			DialogueWidget->HideDialogue();
		}
	}
	if (DiceRollWidgetClass)
	{
		DiceRollWidget = CreateWidget<UILDiceRollWidget>(this, DiceRollWidgetClass);
		if (DiceRollWidget)
		{
			DiceRollWidget->AddToViewport();
			DiceRollWidget->HidePanel();
		}
	}
}

void AILPlayerController::HandleDialogueNodeChanged(FName NodeId)
{
	if (DialogueWidget == nullptr)
	{
		return;
	}

	// 运行器每关由 GameMode 重建，动态获取而非缓存，避免跨关悬挂
	const AILGameModeBase* GameMode = GetWorld() ? Cast<AILGameModeBase>(GetWorld()->GetAuthGameMode()) : nullptr;
	UILDialogueRunner* Runner = GameMode ? GameMode->GetDialogueRunner() : nullptr;

	// NodeId 为 None（对话收尾）或运行器缺失：收起界面并回到探索态输入
	if (NodeId.IsNone() || Runner == nullptr)
	{
		DialogueWidget->HideDialogue();
		SwitchInputContext(FName(TEXT("Exploration")));
		SetUIInputMode(false);
		return;
	}

	FILDialogueNode Node;
	if (!Runner->GetCurrentNode(Node))
	{
		return;
	}

	DialogueWidget->ShowNode(Node, Runner);
	SwitchInputContext(FName(TEXT("Dialogue")));
	SetUIInputMode(true);
}

void AILPlayerController::HandleDiceRollRequested(const FILDiceRoll& Roll)
{
	if (DiceRollWidget == nullptr)
	{
		return;
	}

	// 检定必然由运行器发起，此处取运行器只为把「应用结果」的回调通道传给面板
	const AILGameModeBase* GameMode = GetWorld() ? Cast<AILGameModeBase>(GetWorld()->GetAuthGameMode()) : nullptr;
	UILDialogueRunner* Runner = GameMode ? GameMode->GetDialogueRunner() : nullptr;
	if (Runner == nullptr)
	{
		return;
	}

	DiceRollWidget->PlayRoll(Roll, Runner);
	if (DialogueWidget)
	{
		DialogueWidget->SetRollPending(true);
	}
}

void AILPlayerController::HandleRunFinished(FName EndingId)
{
	// 结算界面懒创建：正常流程一周目只出现一次
	if (ResultWidget == nullptr && ResultWidgetClass)
	{
		ResultWidget = CreateWidget<UILResultWidget>(this, ResultWidgetClass);
		if (ResultWidget)
		{
			ResultWidget->AddToViewport(10);
		}
	}
	if (ResultWidget)
	{
		ResultWidget->ShowResult(EndingId);
	}
	SetUIInputMode(true);
}

void AILPlayerController::HandleDiceRollResolved()
{
	// 解锁选项列表：与 HandleDiceRollRequested 里的 SetRollPending(true) 成对
	if (DialogueWidget)
	{
		DialogueWidget->SetRollPending(false);
	}
}
