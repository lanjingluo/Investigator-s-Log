#pragma once

#include "CoreMinimal.h"
#include "Data/ILDataTypes.h"
#include "UI/ILUserWidgetBase.h"
#include "ILDialogueWidget.generated.h"

class UILDialogueChoiceEntry;
class UILDialogueRunner;
class UImage;
class UTextBlock;
class UVerticalBox;

/**
 * 对话界面：说话人、正文、立绘与选项列表。
 * 数据来源：PlayerController 收到 OnDialogueNodeChanged 后从运行器取节点并调用 ShowNode；
 * 本界面只消费运行器的只读查询，选项点击经条目委托回传索引后转发 SelectChoice。
 */
UCLASS()
class UILDialogueWidget : public UILUserWidgetBase
{
	GENERATED_BODY()

public:
	/**
	 * 显示并刷新一个对话节点。
	 * @param Node    当前节点数据（含选项列表）
	 * @param Runner  对话运行器（弱引用保存，仅用于选项查询与 SelectChoice 转发）
	 */
	void ShowNode(const FILDialogueNode& Node, UILDialogueRunner* Runner);

	/** 收起对话界面（NodeId 为 None / 运行器缺失时由 PC 调用）。 */
	void HideDialogue();

	/**
	 * 设置掷骰等待状态；等待期间锁住选项列表，防止动画期间重复点击。
	 * @param bPending true 表示正在等骰子
	 */
	void SetRollPending(bool bPending);

	/**
	 * 线性节点（无选项）点击面板任意处推进到下一句。
	 * 带选项节点面板不可命中（SelfHitTestInvisible），不会误触发；
	 * 骰子等待期由运行器 bWaitingForRoll 兜底防重复推进。
	 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	/** 构建选项列表（清空后按当前节点逐条创建条目）。 */
	void BuildChoiceEntries();

	/** 条目点击转发：调用运行器 SelectChoice。 */
	UFUNCTION()
	void HandleChoiceClicked(int32 ChoiceIndex);

	/** 立绘占位（具体贴图映射由 WBP 蓝图按 PortraitId 增强）。 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> PortraitImage;

	/** 说话人名。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerNameText;

	/** 正文。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DialogueText;

	/** 选项容器。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ChoiceList;

	/** 选项条目类（编辑器内指定 WBP_ChoiceEntry，须继承 UILDialogueChoiceEntry）。 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UILDialogueChoiceEntry> ChoiceEntryClass;

private:
	/** 当前节点数据快照。 */
	FILDialogueNode CurrentNode;

	/** 对话运行器弱引用（UI 不延长 UObject 生命周期）。 */
	TWeakObjectPtr<UILDialogueRunner> RunnerPtr;
};
