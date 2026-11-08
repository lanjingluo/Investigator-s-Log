#include "UI/ILUserWidgetBase.h"

#include "GameFramework/PlayerController.h"
#include "Runtime/ILGameInstance.h"
#include "Runtime/ILPlayerController.h"
#include "Subsystems/ILGameSessionSubsystem.h"

UILGameSessionSubsystem* UILUserWidgetBase::GetSession() const
{
	// 数据权威在 GameInstance 的模型上，会话子系统只是服务门面，二者都挂在 GameInstance 下
	const APlayerController* PlayerController = GetOwningPlayer();
	const UILGameInstance* InvestigatorGI = PlayerController ? Cast<UILGameInstance>(PlayerController->GetGameInstance()) : nullptr;
	return InvestigatorGI ? InvestigatorGI->GetSession() : nullptr;
}

void UILUserWidgetBase::SetUIInputMode(bool bUIOnly)
{
	// 输入模式归 PlayerController 统一管理（三态 IMC 与鼠标可见性都在那里），UI 只做转发
	if (AILPlayerController* InvestigatorController = Cast<AILPlayerController>(GetOwningPlayer()))
	{
		InvestigatorController->SetUIInputMode(bUIOnly);
	}
}

void UILUserWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	SubscribeSession();
}

void UILUserWidgetBase::NativeDestruct()
{
	UnsubscribeSession();
	Super::NativeDestruct();
}

void UILUserWidgetBase::SubscribeSession()
{
	// 基类默认不订阅；需要监听会话事件的子类自行重写
}

void UILUserWidgetBase::UnsubscribeSession()
{
	// 与 SubscribeSession 成对的空实现
}
