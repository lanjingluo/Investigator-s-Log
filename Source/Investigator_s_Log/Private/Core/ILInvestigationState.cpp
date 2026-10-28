#include "Core/ILInvestigationState.h"

void FILInvestigationState::AddClue(FName ClueId)
{
	// 幂等：重复获得保持 true，不产生副作用
	ClueCollected.Add(ClueId, true);
}

bool FILInvestigationState::HasClue(FName ClueId) const
{
	const bool* Found = ClueCollected.Find(ClueId);
	return Found && *Found;
}

int32 FILInvestigationState::GetCollectedCount() const
{
	int32 Count = 0;
	for (const TPair<FName, bool>& Pair : ClueCollected)
	{
		if (Pair.Value)
		{
			++Count;
		}
	}
	return Count;
}

float FILInvestigationState::GetCompletionRate(int32 TotalClueCount) const
{
	// 总数非法时返回 0，避免除零
	if (TotalClueCount <= 0)
	{
		return 0.0f;
	}
	return FMath::Clamp(GetCollectedCount() / static_cast<float>(TotalClueCount), 0.0f, 1.0f);
}
