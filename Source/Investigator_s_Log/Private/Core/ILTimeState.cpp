#include "Core/ILTimeState.h"

int32 FILTimeState::GetRemainingSteps() const
{
	// 自由探索段返回 -1 表示无限，UI 据此隐藏步数显示
	return bLimitedSteps ? (StepBudget - StepsSpent) : -1;
}

void FILTimeState::BeginScene(int32 InBudget)
{
	if (InBudget <= 0)
	{
		// 自由探索段：不限步，同时清空上一段的计数残留
		bLimitedSteps = false;
		StepBudget = 0;
		StepsSpent = 0;
		return;
	}
	bLimitedSteps = true;
	StepBudget = InBudget;
	StepsSpent = 0;
}

bool FILTimeState::TrySpendStep(int32 Cost)
{
	if (!bLimitedSteps)
	{
		// 自由段恒成功且不计数
		return true;
	}
	if (StepsSpent + Cost > StepBudget)
	{
		// 步数不足：返回 false，由调用方阻止该交互并提示
		return false;
	}
	StepsSpent += Cost;
	return true;
}

void FILTimeState::ApplyFaint(int32 WastedHoursPerFaint)
{
	++FaintCount;
	// 只记账不提示：昏迷的时间代价属于隐藏机制，结算界面才允许读取
	WastedHours += FMath::Max(WastedHoursPerFaint, 0);
}
