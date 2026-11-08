#include "UI/ILDialogueWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/ILGameSettings.h"
#include "Subsystems/ILDialogueRunner.h"
#include "Types/SlateEnums.h"
#include "UI/ILDialogueChoiceEntry.h"

void UILDialogueWidget::ShowNode(const FILDialogueNode& Node, UILDialogueRunner* Runner)
{
	RunnerPtr = Runner;
	CurrentNode = Node;

	SetVisibility(Node.Choices.Num() > 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Visible);

	// 说话人与正文直接来自表行；说话人 ID 先以原文显示，本地化映射表接入后替换
	if (SpeakerNameText)
	{
		SpeakerNameText->SetText(FText::FromName(Node.SpeakerId));
	}
	if (DialogueText)
	{
		DialogueText->SetText(Node.Text);
	}
	// 立绘键为空则隐藏占位；PortraitId → 贴图的映射由 WBP 蓝图按命名约定增强
	if (PortraitImage)
	{
		PortraitImage->SetVisibility(Node.PortraitId.IsNone() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	BuildChoiceEntries();
}

void UILDialogueWidget::HideDialogue()
{
	SetVisibility(ESlateVisibility::Collapsed);
	if (ChoiceList)
	{
		ChoiceList->ClearChildren();
	}
	RunnerPtr = nullptr;
}

void UILDialogueWidget::SetRollPending(bool bPending)
{
	// 等待骰子动画期间锁选项交互；真正的防重复由运行器的 bWaitingForRoll 兜底
	if (ChoiceList)
	{
		ChoiceList->SetVisibility(bPending ? ESlateVisibility::HitTestInvisible : ESlateVisibility::SelfHitTestInvisible);
	}
}

void UILDialogueWidget::BuildChoiceEntries()
{
	if (ChoiceList == nullptr)
	{
		return;
	}
	ChoiceList->ClearChildren();

	UILDialogueRunner* Runner = RunnerPtr.Get();
	if (Runner == nullptr)
	{
		return;
	}

	const UILGameSettings* Settings = GetDefault<UILGameSettings>();
	for (int32 Index = 0; Index < CurrentNode.Choices.Num(); ++Index)
	{
		const FILDialogueChoice& Choice = CurrentNode.Choices[Index];
		UILDialogueChoiceEntry* Entry = CreateWidget<UILDialogueChoiceEntry>(this, ChoiceEntryClass);
		if (Entry == nullptr)
		{
			continue;
		}

		const bool bEnabled = Runner->IsChoiceEnabled(Choice);
		// 成功率预估：把全局大成功/大失败配置注入检定工具，保证与实际掷骰判定一致
		const float Chance = Runner->GetChoiceSuccessChance(Choice);
		Entry->Setup(Choice, Index, bEnabled, Chance);
		Entry->OnChoiceClicked.AddDynamic(this, &UILDialogueWidget::HandleChoiceClicked);
		ChoiceList->AddChildToVerticalBox(Entry);
	}
}

void UILDialogueWidget::HandleChoiceClicked(int32 ChoiceIndex)
{
	if (UILDialogueRunner* Runner = RunnerPtr.Get())
	{
		Runner->SelectChoice(ChoiceIndex);
	}
}

FReply UILDialogueWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 线性节点点击推进：Advance 对带选项节点与骰子等待期内部直接忽略
	if (UILDialogueRunner* Runner = RunnerPtr.Get())
	{
		Runner->Advance();
	}
	return FReply::Handled();
}
