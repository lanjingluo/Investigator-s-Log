#include "Core/ILAttributes.h"

int32 FILAttributes::GetMaxHP() const
{
	// 策划公式：HP = (STR + CON) / 2 + 2，整数除法天然向下取整
	return (STR + CON) / 2 + 2;
}

int32 FILAttributes::GetMaxSAN() const
{
	// 策划公式：SAN = POW × 3
	return POW * 3;
}

int32 FILAttributes::GetStepBudget(int32 Base, float Divisor, int32 MinValue, int32 MaxValue) const
{
	// Divisor 近零时退化为纯基数，避免除零崩溃
	const int32 IntBonus = FMath::IsNearlyZero(Divisor) ? 0 : FMath::FloorToInt((INT - 10) / Divisor);
	return FMath::Clamp(Base + IntBonus, MinValue, MaxValue);
}

int32 FILSkillSet::GetSkill(EILSkill Skill, int32 DefaultValue) const
{
	const int32* Found = Values.Find(Skill);
	return Found ? *Found : DefaultValue;
}

void FILSkillSet::AddSkillPoints(EILSkill Skill, int32 Delta, int32 MaxValue)
{
	// 未配置的技能从 0 起累加（不套用 DefaultValue），保证加点语义纯粹
	const int32 Current = Values.FindRef(Skill);
	Values.Add(Skill, FMath::Clamp(Current + Delta, 0, MaxValue));
}
