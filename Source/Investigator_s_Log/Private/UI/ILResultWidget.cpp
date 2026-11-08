#include "UI/ILResultWidget.h"

#include "Components/TextBlock.h"
#include "Core/ILPlayerModel.h"
#include "Internationalization/Text.h"
#include "Subsystems/ILGameSessionSubsystem.h"
#include "Types/SlateEnums.h"

void UILResultWidget::ShowResult(FName EndingId)
{
	SetVisibility(ESlateVisibility::Visible);

	const UILGameSessionSubsystem* Session = GetSession();
	const FILPlayerModel* Model = Session ? &Session->GetModel() : nullptr;

	if (EndingNameText)
	{
		// 结局名先以 ID 原文显示；结局文案映射表接入后替换为本地化文本
		EndingNameText->SetText(FText::FromName(EndingId));
	}

	if (Model)
	{
		// 策划要求的「唯一点破隐藏机制」位置：昏迷次数与被浪费的时间
		if (FaintCountText)
		{
			FaintCountText->SetText(FText::Format(
				NSLOCTEXT("IL", "ResultFaintFmt", "昏迷次数：{0}"),
				FText::AsNumber(Model->Time.FaintCount)));
		}
		if (WastedTimeText)
		{
			WastedTimeText->SetText(FText::Format(
				NSLOCTEXT("IL", "ResultWastedFmt", "因昏迷浪费的时间：{0} 小时"),
				FText::AsNumber(Model->Time.WastedHours)));
		}

		// 信息完整度：已收集线索 / 当前周目线索总数
		if (CompletionRateText && Session)
		{
			const int32 Percent = FMath::RoundToInt(Session->GetCompletionRate() * 100.f);
			CompletionRateText->SetText(FText::Format(
				NSLOCTEXT("IL", "ResultCompletionFmt", "信息完整度：{0}%"),
				FText::AsNumber(Percent)));
		}
	}
}
