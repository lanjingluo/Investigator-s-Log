#include "Misc/AutomationTest.h"

#include "Core/ILAttributes.h"
#include "Core/ILCheckSystem.h"
#include "Core/ILTimeState.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 属性派生值测试：HP/SAN 公式与步数池区间。
 * 覆盖策划数值框架的核心公式边界。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FILCoreAttributesTest,
	"Investigator.Logic.Core.Attributes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FILCoreAttributesTest::RunTest(const FString& Parameters)
{
	FILAttributes Attr;
	Attr.STR = 11;
	Attr.CON = 13;
	Attr.POW = 12;
	Attr.INT = 10;

	// HP = (STR + CON) / 2 + 2 = 14（整数除法向下取整）
	TestEqual(TEXT("MaxHP 应为 (11+13)/2+2 = 14"), Attr.GetMaxHP(), 14);
	// SAN = POW × 3 = 36
	TestEqual(TEXT("MaxSAN 应为 12*3 = 36"), Attr.GetMaxSAN(), 36);
	// 步数池应落在策划常见区间 [8, 25]
	TestTrue(TEXT("INT=10 时步数池应在 8-25 区间内"),
		Attr.GetStepBudget() >= 8 && Attr.GetStepBudget() <= 25);

	return true;
}

/**
 * 检定测试：难度折算（向下取整）与 d100 判定边界（96-100 大失败）。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FILCoreCheckTest,
	"Investigator.Logic.Core.Check",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FILCoreCheckTest::RunTest(const FString& Parameters)
{
	// 难度折算：向下取整
	TestEqual(TEXT("困难 65 → 32"), UILCheckSystem::GetEffectiveTarget(65, EILDifficulty::Hard), 32);
	TestEqual(TEXT("极难 65 → 13"), UILCheckSystem::GetEffectiveTarget(65, EILDifficulty::Extreme), 13);
	// 极低技能打极难取整为 0，应被钳到 1（保证永远有成功可能）
	TestEqual(TEXT("目标值最小为 1"), UILCheckSystem::GetEffectiveTarget(3, EILDifficulty::Extreme), 1);

	// 固定种子循环：验证点数范围与 96-100 大失败边界
	// 技能 99 打普通（目标 99）：只有 96-100 会失败，其中 96-99 必须判为大失败
	FRandomStream Stream(12345);
	for (int32 i = 0; i < 2000; ++i)
	{
		FILDiceRoll Roll;
		UILCheckSystem::RollCheck(99, EILDifficulty::Regular, Stream, Roll);
		TestTrue(TEXT("掷骰结果在 1-100 之间"), Roll.RawRoll >= 1 && Roll.RawRoll <= 100);
		if (Roll.RawRoll >= 96)
		{
			TestEqual(TEXT("96-100 必须是 CriticalFailure"),
				static_cast<int32>(Roll.Result), static_cast<int32>(EILRollResult::CriticalFailure));
		}
	}
	return true;
}

/**
 * 时间/步数/昏迷测试：限步消耗、自由段语义、昏迷记账（隐藏机制数据源）。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FILCoreTimeTest,
	"Investigator.Logic.Core.Time",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FILCoreTimeTest::RunTest(const FString& Parameters)
{
	FILTimeState Time;

	// 限步段：3 步池，第 4 次消耗必须被拒绝
	Time.BeginScene(3);
	TestTrue(TEXT("第 1 步消耗成功"), Time.TrySpendStep());
	TestTrue(TEXT("第 2 步消耗成功"), Time.TrySpendStep());
	TestTrue(TEXT("第 3 步消耗成功"), Time.TrySpendStep());
	TestFalse(TEXT("步数耗尽后必须失败"), Time.TrySpendStep());
	TestEqual(TEXT("剩余步数为 0"), Time.GetRemainingSteps(), 0);

	// 自由段：恒成功且不计数，剩余步数返回 -1 表示无限
	Time.BeginScene(0);
	TestTrue(TEXT("自由段恒成功"), Time.TrySpendStep());
	TestEqual(TEXT("自由段剩余步数返回 -1"), Time.GetRemainingSteps(), -1);

	// 昏迷记账：次数与浪费时间同步累加，终章难度加成等于昏迷次数
	Time.ApplyFaint(2);
	Time.ApplyFaint(2);
	TestEqual(TEXT("昏迷次数应为 2"), Time.FaintCount, 2);
	TestEqual(TEXT("浪费小时数应为 4"), Time.WastedHours, 4);
	TestEqual(TEXT("终章难度加成等于昏迷次数"), Time.GetFinaleDifficultyBonus(), 2);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
