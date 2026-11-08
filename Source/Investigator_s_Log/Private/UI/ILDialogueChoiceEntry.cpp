#include "UI/ILDialogueChoiceEntry.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Core/ILCheckSystem.h"
#include "Internationalization/Text.h"
#include "UObject/Class.h"

void UILDialogueChoiceEntry::Setup(const FILDialogueChoice& InChoice, int32 InIndex, bool bEnabled, float SuccessChance)
{
	ChoiceIndex = InIndex;

	if (ChoiceText)
	{
		if (InChoice.bRequiresCheck)
		{
			// 徽标拼装：「[侦查·困难] 正文（成功率 48%）」；难度中文取自枚举 DisplayName
			const FText SkillText = UILCheckSystem::GetSkillText(InChoice.Skill);
			const FText DifficultyText = UEnum::GetDisplayValueAsText(InChoice.Difficulty);
			const int32 Percent = FMath::RoundToInt(SuccessChance * 100.f);
			ChoiceText->SetText(FText::Format(
				NSLOCTEXT("IL", "ChoiceBadgeFmt", "[{0}·{1}] {2}（成功率 {3}%）"),
				SkillText,
				DifficultyText,
				InChoice.Text,
				FText::AsNumber(Percent)));
		}
		else
		{
			// 无检定的线性选项只显示正文
			ChoiceText->SetText(InChoice.Text);
		}
	}

	// 前置线索/标记位不满足或已选过的一次性选项置灰
	if (ChoiceButton)
	{
		ChoiceButton->SetIsEnabled(bEnabled);
	}
}

void UILDialogueChoiceEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (ChoiceButton)
	{
		ChoiceButton->OnClicked.AddDynamic(this, &UILDialogueChoiceEntry::HandleClicked);
	}
}

void UILDialogueChoiceEntry::HandleClicked()
{
	if (ChoiceIndex != INDEX_NONE)
	{
		OnChoiceClicked.Broadcast(ChoiceIndex);
	}
}
