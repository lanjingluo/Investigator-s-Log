#include "Core/ILPartyState.h"

void FILPartyMemberState::ApplyHP(int32 Delta)
{
	// clamp 到 [0, MaxHP]；队友 HP 归零不进入昏迷流程，由上层按剧情标记 bIsGone
	CurrentHP = FMath::Clamp(CurrentHP + Delta, 0, MaxHP);
}

void FILPartyMemberState::ApplySAN(int32 Delta)
{
	CurrentSAN = FMath::Clamp(CurrentSAN + Delta, 0, MaxSAN);
	// 归零置癫狂；bool 置位天然幂等，重复掉 SAN 不会重复触发副作用
	if (CurrentSAN <= 0)
	{
		bIsBroken = true;
	}
}
