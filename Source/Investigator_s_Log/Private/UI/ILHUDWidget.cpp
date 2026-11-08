#include "UI/ILHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Core/ILPlayerModel.h"
#include "Subsystems/ILGameSessionSubsystem.h"
#include "Types/SlateEnums.h"

void UILHUDWidget::SubscribeSession()
{
	if (UILGameSessionSubsystem* Session = GetSession())
	{
		Session->OnStatChanged.AddDynamic(this, &UILHUDWidget::HandleStatChanged);
	}
	// 建立时模型可能已有进度（读档/中途入关），立即按当前值补齐一次
	RefreshFromModel();
}

void UILHUDWidget::UnsubscribeSession()
{
	if (UILGameSessionSubsystem* Session = GetSession())
	{
		Session->OnStatChanged.RemoveDynamic(this, &UILHUDWidget::HandleStatChanged);
	}
}

void UILHUDWidget::RefreshFromModel()
{
	if (const UILGameSessionSubsystem* Session = GetSession())
	{
		const FILPlayerModel& Model = Session->GetModel();
		HandleStatChanged(Model.CurrentHP, Model.CurrentSAN, Model.Time.GetRemainingSteps());
	}
}

void UILHUDWidget::HandleStatChanged(int32 CurrentHP, int32 CurrentSAN, int32 RemainingSteps)
{
	const UILGameSessionSubsystem* Session = GetSession();
	const FILPlayerModel* Model = Session ? &Session->GetModel() : nullptr;

	// 条形图按最大值归一化；上限来自属性派生值（HP=(STR+CON)/2+2，SAN=POW*3）
	if (HPBar)
	{
		const float MaxHP = Model ? static_cast<float>(FMath::Max(Model->Attributes.GetMaxHP(), 1)) : 1.f;
		HPBar->SetPercent(FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f));
	}
	if (SANBar)
	{
		const float MaxSAN = Model ? static_cast<float>(FMath::Max(Model->Attributes.GetMaxSAN(), 1)) : 1.f;
		SANBar->SetPercent(FMath::Clamp(CurrentSAN / MaxSAN, 0.f, 1.f));
	}

	// 剩余步数约定：>=0 为限步段，-1 表示自由探索（隐藏整个步数面板）
	if (StepPanel)
	{
		const bool bLimited = RemainingSteps >= 0;
		StepPanel->SetVisibility(bLimited ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (bLimited && StepText)
		{
			StepText->SetText(FText::AsNumber(RemainingSteps));
		}
	}
}
