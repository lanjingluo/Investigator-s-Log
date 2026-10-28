#include "Core/ILCheckSystem.h"

#include "Core/ILTypes.h"

#define LOCTEXT_NAMESPACE "ILCheck"

int32 UILCheckSystem::GetEffectiveTarget(int32 SkillValue, EILDifficulty Difficulty)
{
	int32 Target = SkillValue;
	switch (Difficulty)
	{
	case EILDifficulty::Hard:
		// 困难：向下取整（65 -> 32）
		Target = FMath::FloorToInt(SkillValue / 2.0f);
		break;
	case EILDifficulty::Extreme:
		// 极难：向下取整（65 -> 13）
		Target = FMath::FloorToInt(SkillValue / 5.0f);
		break;
	case EILDifficulty::Regular:
	default:
		break;
	}
	// 目标值下限为 1：即使 3 技能打极难（0.6 -> 0）也保留 1 点成功空间
	return FMath::Max(Target, 1);
}

float UILCheckSystem::GetSuccessChance(int32 SkillValue, EILDifficulty Difficulty,
	bool bUseCriticalSuccess, int32 CriticalFailureMin)
{
	// 大成功是成功的子集，不改变成功计数；大失败边界会截掉高段点数，
	// 因此成功点数空间 = [1, min(目标值, 大失败下限-1)]。bUseCriticalSuccess 仅保持与 RollCheck 的签名一致。
	const int32 Target = GetEffectiveTarget(SkillValue, Difficulty);
	const int32 SuccessCap = FMath::Min(Target, CriticalFailureMin - 1);
	const int32 SuccessCount = FMath::Max(SuccessCap, 0);
	return SuccessCount / 100.0f;
}

EILRollResult UILCheckSystem::RollCheck(int32 SkillValue, EILDifficulty Difficulty,
	FRandomStream& Stream, FILDiceRoll& OutRoll,
	bool bUseCriticalSuccess, int32 CriticalSuccessMax, int32 CriticalFailureMin)
{
	const int32 Target = GetEffectiveTarget(SkillValue, Difficulty);
	const int32 Roll = Stream.RandRange(1, 100);

	OutRoll.Skill = EILSkill::Investigation; // 技能标识由调用方按上下文覆盖
	OutRoll.Difficulty = Difficulty;
	OutRoll.RawSkillValue = SkillValue;
	OutRoll.EffectiveTarget = Target;
	OutRoll.RawRoll = Roll;

	if (Roll >= CriticalFailureMin)
	{
		// 大失败优先：目标值再高也拦不住 96-100
		OutRoll.Result = EILRollResult::CriticalFailure;
	}
	else if (bUseCriticalSuccess && Roll <= CriticalSuccessMax && Roll <= Target)
	{
		// 大成功需要「点数足够低」且「本身确实成功」双重条件
		OutRoll.Result = EILRollResult::CriticalSuccess;
	}
	else
	{
		OutRoll.Result = (Roll <= Target) ? EILRollResult::Success : EILRollResult::Failure;
	}
	return OutRoll.Result;
}

FText UILCheckSystem::GetResultText(EILRollResult Result)
{
	switch (Result)
	{
	case EILRollResult::CriticalSuccess:	return LOCTEXT("ResultCriticalSuccess", "大成功");
	case EILRollResult::Success:			return LOCTEXT("ResultSuccess", "成功");
	case EILRollResult::Failure:			return LOCTEXT("ResultFailure", "失败");
	case EILRollResult::CriticalFailure:	return LOCTEXT("ResultCriticalFailure", "大失败");
	default:								break;
	}
	return FText::GetEmpty();
}

FText UILCheckSystem::GetSkillText(EILSkill Skill)
{
	switch (Skill)
	{
	case EILSkill::Investigation:	return LOCTEXT("SkillInvestigation", "侦查");
	case EILSkill::Psychology:		return LOCTEXT("SkillPsychology", "心理学");
	case EILSkill::Persuasion:		return LOCTEXT("SkillPersuasion", "说服");
	case EILSkill::Occultism:		return LOCTEXT("SkillOccultism", "神秘学");
	case EILSkill::LibraryUse:		return LOCTEXT("SkillLibraryUse", "图书馆使用");
	case EILSkill::Climb:			return LOCTEXT("SkillClimb", "攀爬");
	case EILSkill::Stealth:			return LOCTEXT("SkillStealth", "潜行");
	case EILSkill::Fighting:		return LOCTEXT("SkillFighting", "格斗");
	default:						break;
	}
	return FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE
