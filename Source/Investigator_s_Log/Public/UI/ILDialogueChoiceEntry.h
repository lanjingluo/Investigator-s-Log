#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ILDataTypes.h"
#include "ILDialogueChoiceEntry.generated.h"

class UButton;
class UTextBlock;

/** 选项点击事件（父对话界面订阅后转发给对话运行器）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FILOnChoiceEntryClicked, int32, ChoiceIndex);

/**
 * 对话选项条目：一条可点击的选项（含技能·难度徽标与置灰状态）。
 * 由 UILDialogueWidget 按节点选项数动态创建；本条目不认识运行器，
 * 点击只广播索引，保持「UI 不反向写数据」的单向数据流。
 */
UCLASS()
class UILDialogueChoiceEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 填充选项内容与状态。
	 * 徽标由本函数自动拼装「[技能·难度] 正文（成功率 xx%）」，策划不在表里手写；
	 * bRequiresCheck 为 false 的线性选项只显示正文。
	 * @param InChoice       选项数据
	 * @param InIndex        选项索引（回传给父界面的下标，与 GetAvailableChoices 对齐）
	 * @param bEnabled       前置条件是否满足（不满足则按钮置灰）
	 * @param SuccessChance  预估成功率（0-1）
	 */
	void Setup(const FILDialogueChoice& InChoice, int32 InIndex, bool bEnabled, float SuccessChance);

	/** 点击广播（参数为选项索引）。 */
	UPROPERTY(BlueprintAssignable, Category = "Choice")
	FILOnChoiceEntryClicked OnChoiceClicked;

protected:
	virtual void NativeConstruct() override;

	/** 按钮 OnClicked 转发为带索引的委托广播。 */
	UFUNCTION()
	void HandleClicked();

	/** 选项按钮。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChoiceButton;

	/** 选项文本（含徽标）。 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ChoiceText;

private:
	/** 本条目对应的选项索引；未 Setup 前为 INDEX_NONE，点击不广播。 */
	int32 ChoiceIndex = INDEX_NONE;
};
