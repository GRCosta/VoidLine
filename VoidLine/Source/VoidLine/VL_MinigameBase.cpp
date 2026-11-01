#include "VL_MinigameBase.h"

#include "HAL/PlatformTime.h"

float UVL_MinigameBase::GetElapsed() const
{
	return bStarted ? static_cast<float>(FPlatformTime::Seconds() - StartTime) : 0.f;
}

void UVL_MinigameBase::Start()
{
	bStarted = true;
	bFinished = false;
	bSuccess = false;
	Hits = 0;
	Total = 0;
	StartTime = FPlatformTime::Seconds();
	OnStart();
}

void UVL_MinigameBase::Update()
{
	if (bStarted && !bFinished)
	{
		OnUpdate(GetElapsed());
	}
}

void UVL_MinigameBase::SendInput(EVL_MinigameInput Input)
{
	if (bStarted && !bFinished)
	{
		OnInput(Input);
	}
}

void UVL_MinigameBase::Complete(bool bInSuccess, int32 InHits, int32 InTotal)
{
	bFinished = true;
	bSuccess = bInSuccess;
	Hits = InHits;
	Total = InTotal;
}