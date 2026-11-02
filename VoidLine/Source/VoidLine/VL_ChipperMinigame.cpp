#include "VL_ChipperMinigame.h"

#include "Engine/Engine.h"
#include "GameFramework/HUD.h"

void UVL_ChipperMinigame::OnStart()
{
	Logs.Reset();
	HitCount = 0;

	const int32 Lanes = FMath::Max(LaneCount, 2);
	TargetLane = Lanes / 2;
	ChipperPos = static_cast<float>(TargetLane);
	LastElapsed = 0.f;

	const int32 Count = FMath::Max(LogCount, 1);
	const float First = FallTime + 0.3f;
	const float Last = FMath::Max(Duration - 1.f, First);
	const float BaseGap = (Count > 1) ? (Last - First) / (Count - 1) : 0.f;

	// Build the schedule so every log is reachable: the gap between two logs is never
	// shorter than the time the chipper needs to slide between their lanes.
	float Time = First;
	int32 PreviousLane = TargetLane;

	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Lane = FMath::RandRange(0, Lanes - 1);

		if (i > 0)
		{
			const float Jitter = FMath::FRandRange(-0.15f, 0.15f) * BaseGap;
			const float TravelTime = FMath::Abs(Lane - PreviousLane) / ChipperSpeed + 0.2f;
			Time += FMath::Max(BaseGap + Jitter, TravelTime);
		}

		if (Time > Duration - 0.5f)
		{
			break;
		}

		FVL_ChipperLog Log;
		Log.Lane = Lane;
		Log.HitTime = Time;
		Logs.Add(Log);
		PreviousLane = Lane;
	}
}

void UVL_ChipperMinigame::OnInput(EVL_MinigameInput Input)
{
	const int32 MaxLane = FMath::Max(LaneCount, 2) - 1;

	if (Input == EVL_MinigameInput::Left)
	{
		TargetLane = FMath::Max(TargetLane - 1, 0);
	}
	else
	{
		TargetLane = FMath::Min(TargetLane + 1, MaxLane);
	}
}

void UVL_ChipperMinigame::OnUpdate(float Elapsed)
{
	const float DeltaTime = FMath::Max(Elapsed - LastElapsed, 0.f);
	LastElapsed = Elapsed;

	// Slide the chipper toward its target lane at a constant speed.
	ChipperPos = FMath::FInterpConstantTo(ChipperPos, static_cast<float>(TargetLane), DeltaTime, ChipperSpeed);

	int32 FallingCount = 0;
	for (FVL_ChipperLog& Log : Logs)
	{
		if (Log.State != 0)
		{
			continue;
		}

		if (Elapsed >= Log.HitTime)
		{
			if (FMath::Abs(ChipperPos - static_cast<float>(Log.Lane)) <= CatchTolerance)
			{
				Log.State = 1;
				++HitCount;
			}
			else
			{
				Log.State = 2;
			}
		}
		else
		{
			++FallingCount;
		}
	}

	if (FallingCount == 0 || Elapsed >= Duration)
	{
		Complete(HitCount * PiecesPerLog);
	}
}

void UVL_ChipperMinigame::Draw(AHUD* HUD, float CanvasWidth, float CanvasHeight) const
{
	if (!HUD)
	{
		return;
	}

	const float Elapsed = GetElapsed();
	const int32 Lanes = FMath::Max(LaneCount, 2);

	// Layout
	const float LaneW = 110.f;
	const float LaneGap = 16.f;
	const float LanesWidth = Lanes * LaneW + (Lanes - 1) * LaneGap;
	const float PanelW = LanesWidth + 60.f;
	const float PanelH = 520.f;
	const float PanelX = CanvasWidth - PanelW - 80.f;
	const float PanelY = (CanvasHeight - PanelH) * 0.5f;
	const float LanesX = PanelX + 30.f;
	const float TopY = PanelY + 80.f;
	const float CatchY = PanelY + 390.f;
	const float LogH = 34.f;
	const float FallDistance = CatchY - TopY;

	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), PanelX, PanelY, PanelW, PanelH);

	UFont* Font = GEngine->GetMediumFont();
	const float TimeLeft = FMath::Max(Duration - Elapsed, 0.f);

	HUD->DrawText(TEXT("WOOD CHIPPER"), FLinearColor::White, PanelX + 20.f, PanelY + 15.f, Font, 1.2f);
	HUD->DrawText(FString::Printf(TEXT("Time %.1f   Wood %d"), TimeLeft, HitCount * PiecesPerLog),
		FLinearColor(1.f, 0.85f, 0.4f), PanelX + 20.f, PanelY + 45.f, Font, 1.f);

	for (int32 Lane = 0; Lane < Lanes; ++Lane)
	{
		const float X = LanesX + Lane * (LaneW + LaneGap);
		HUD->DrawRect(FLinearColor(0.12f, 0.12f, 0.12f, 0.9f), X, TopY, LaneW, FallDistance + 90.f);
	}

	// Logs: caught ones are gone (chipped), missed ones keep falling in red.
	for (const FVL_ChipperLog& Log : Logs)
	{
		if (Log.State == 1)
		{
			continue;
		}

		const float Progress = 1.f - (Log.HitTime - Elapsed) / FallTime;
		if (Progress < 0.f || Progress > 1.3f)
		{
			continue;
		}

		const float X = LanesX + Log.Lane * (LaneW + LaneGap);
		const float Y = TopY + Progress * FallDistance;
		const FLinearColor Color = (Log.State == 0)
			? FLinearColor(0.5f, 0.32f, 0.14f, 1.f)   // log
			: FLinearColor(0.7f, 0.1f, 0.1f, 0.6f);   // missed

		HUD->DrawRect(Color, X + 8.f, Y - LogH * 0.5f, LaneW - 16.f, LogH);
	}

	// The chipper, drawn at its current sliding position.
	const float ChipperX = LanesX + ChipperPos * (LaneW + LaneGap);
	HUD->DrawRect(FLinearColor(0.85f, 0.45f, 0.1f, 1.f), ChipperX + 4.f, CatchY - 6.f, LaneW - 8.f, 46.f);
	HUD->DrawRect(FLinearColor(0.95f, 0.9f, 0.7f, 1.f), ChipperX + 4.f, CatchY - 6.f, LaneW - 8.f, 8.f);

	HUD->DrawText(TEXT("LB  <     >  RB"), FLinearColor::White, PanelX + PanelW * 0.5f - 80.f, CatchY + 100.f, Font, 1.2f);
}