#include "VL_WorldStateSubsystem.h"

void UVL_WorldStateSubsystem::AdvanceNight()
{
	SetNight(Night + 1);
}

void UVL_WorldStateSubsystem::SetNight(int32 NewNight)
{
	Night = FMath::Max(NewNight, 1);
	OnNightChanged.Broadcast(Night);
}

int32 UVL_WorldStateSubsystem::GetSeedFor(int32 Salt) const
{
	uint32 Hash = HashCombine(GetTypeHash(WorldSeed), GetTypeHash(Night));
	Hash = HashCombine(Hash, GetTypeHash(Salt));
	return static_cast<int32>(Hash);
}