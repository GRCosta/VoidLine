#include "VL_TimingLanesMinigame.h"

#include "Engine/Engine.h"
#include "GameFramework/HUD.h"

void UVL_TimingLanesMinigame::OnStart()
{
	Pieces.Reset();
	HitCount = 0;

	const int32 Count = FMath::Max(PieceCount, 1);
	const float First = FallTime + 0.3f;
	const float Last = FMath::Max(Duration - 1.f, First);
	const float Spacing = (Count > 1) ? (Last - First) / (Count - 1) : 0.f;

	for (int32 i = 0; i < Count; ++i)
	{
		FVL_TimingPiece Piece;
		Piece.Lane = FMath::RandRange(0, 1);
		const float Jitter = (i > 0 && i < Count - 1) ? FMath::FRandRange(-0.15f, 0.15f) * Spacing : 0.f;
		Piece.HitTime = First + Spacing * i + Jitter;
		Pieces.Add(Piece);
	}
}

void UVL_TimingLanesMinigame::OnInput(EVL_MinigameInput Input)
{
	const int32 Lane = (Input == EVL_MinigameInput::Left) ? 0 : 1;
	const float Now = GetElapsed();

	int32 BestIndex = INDEX_NONE;
	float BestDelta = HitWindow;

	for (int32 i = 0; i < Pieces.Num(); ++i)
	{
		const FVL_TimingPiece& Piece = Pieces[i];
		if (Piece.State != 0 || Piece.Lane != Lane)
		{
			continue;
		}
		const float Delta = FMath::Abs(Piece.HitTime - Now);
		if (Delta <= BestDelta)
		{
			BestDelta = Delta;
			BestIndex = i;
		}
	}

	if (BestIndex != INDEX_NONE)
	{
		Pieces[BestIndex].State = 1;
		++HitCount;
	}
}

void UVL_TimingLanesMinigame::OnUpdate(float Elapsed)
{
	int32 PendingCount = 0;

	for (FVL_TimingPiece& Piece : Pieces)
	{
		if (Piece.State == 0)
		{
			if (Elapsed > Piece.HitTime + HitWindow)
			{
				Piece.State = 2;
			}
			else
			{
				++PendingCount;
			}
		}
	}

	if (PendingCount == 0 || Elapsed >= Duration)
	{
		Complete(HitCount * PiecesPerCatch);
	}
}

void UVL_TimingLanesMinigame::Draw(AHUD* HUD, float CanvasWidth, float CanvasHeight) const
{
	if (!HUD)
	{
		return;
	}

	const float Elapsed = GetElapsed();

	// Layout
	const float PanelW = 360.f;
	const float PanelH = 520.f;
	const float PanelX = CanvasWidth - PanelW - 80.f;
	const float PanelY = (CanvasHeight - PanelH) * 0.5f;
	const float LaneW = 140.f;
	const float LaneGap = 20.f;
	const float LanesX = PanelX + (PanelW - (2.f * LaneW + LaneGap)) * 0.5f;
	const float TopY = PanelY + 80.f;
	const float CatchY = PanelY + 390.f;
	const float PieceH = 28.f;
	const float FallDistance = CatchY - TopY;

	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), PanelX, PanelY, PanelW, PanelH);

	UFont* Font = GEngine->GetMediumFont();
	const float TimeLeft = FMath::Max(Duration - Elapsed, 0.f);

	HUD->DrawText(TEXT("COLLECTING"), FLinearColor::White, PanelX + 20.f, PanelY + 15.f, Font, 1.2f);
	HUD->DrawText(FString::Printf(TEXT("Time %.1f   Collected %d"), TimeLeft, HitCount * PiecesPerCatch),
		FLinearColor(1.f, 0.85f, 0.4f), PanelX + 20.f, PanelY + 45.f, Font, 1.f);

	const float BandH = 2.f * (HitWindow / FallTime) * FallDistance;

	for (int32 Lane = 0; Lane < 2; ++Lane)
	{
		const float X = LanesX + Lane * (LaneW + LaneGap);

		HUD->DrawRect(FLinearColor(0.12f, 0.12f, 0.12f, 0.9f), X, TopY, LaneW, FallDistance + 90.f);
		HUD->DrawRect(FLinearColor(0.2f, 0.9f, 0.3f, 0.25f), X, CatchY - BandH * 0.5f, LaneW, BandH);
		HUD->DrawRect(FLinearColor::White, X, CatchY - 2.f, LaneW, 4.f);

		HUD->DrawText(Lane == 0 ? TEXT("LB") : TEXT("RB"), FLinearColor::White,
			X + LaneW * 0.5f - 12.f, CatchY + 100.f, Font, 1.2f);
	}

	for (const FVL_TimingPiece& Piece : Pieces)
	{
		if (Piece.State == 1)
		{
			continue; // caught
		}

		const float Progress = 1.f - (Piece.HitTime - Elapsed) / FallTime;
		if (Progress < 0.f || Progress > 1.3f)
		{
			continue;
		}

		const float X = LanesX + Piece.Lane * (LaneW + LaneGap);
		const float Y = TopY + Progress * FallDistance;
		const FLinearColor Color = (Piece.State == 0)
			? FLinearColor(0.55f, 0.35f, 0.15f, 1.f)   // wood
			: FLinearColor(0.7f, 0.1f, 0.1f, 0.6f);    // missed

		HUD->DrawRect(Color, X + 10.f, Y - PieceH * 0.5f, LaneW - 20.f, PieceH);
	}
}