#include "Core/ILPlayerModel.h"

void FILPlayerModel::InitializeFromAttributes()
{
	// 新周目初始化：按属性公式回满双状态条
	CurrentHP = Attributes.GetMaxHP();
	CurrentSAN = Attributes.GetMaxSAN();
}

void FILPlayerModel::ApplyHP(int32 Delta)
{
	// clamp 到 [0, MaxHP]；归零的昏迷副作用（Time.ApplyFaint）由会话子系统统一处理，
	// 保证「掉血」与「昏迷记账」职责分离
	CurrentHP = FMath::Clamp(CurrentHP + Delta, 0, Attributes.GetMaxHP());
}

void FILPlayerModel::ApplySAN(int32 Delta)
{
	CurrentSAN = FMath::Clamp(CurrentSAN + Delta, 0, Attributes.GetMaxSAN());
}

FILPartyMemberState* FILPlayerModel::FindPartyMember(FName MemberId)
{
	for (FILPartyMemberState& Member : Party)
	{
		if (Member.MemberId == MemberId)
		{
			return &Member;
		}
	}
	return nullptr;
}
