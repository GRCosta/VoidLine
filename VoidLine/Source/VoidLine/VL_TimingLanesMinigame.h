#pragma once

#include "CoreMinimal.h"
#include "VL_MinigameBase.h"
#include "VL_TimingLanesMinigame.generated.h"

struct FVL_TimingPiece
{
	int32 Lane = 0;
	float HitTime = 0.f; // seconds since the minigame started
	uint8 State = 0;     // 0 = pending, 1 = caught, 2 = missed
};

// Two lanes of falling pieces. Press LB / RB when a piece reaches the catch line.
UCLASS(meta = (DisplayName = "Timing Lanes"))
class VOIDLINE_API UVL_TimingLanesMinigame : public UVL_MinigameBase
{
	GENERATED_BODY()

public:
	virtual void Draw(AHUD* HUD, float CanvasWidth, float CanvasHeight) const override;

protected:
	virtual void OnStart() override;
	virtual void OnUpdate(float Elapsed) override;
	virtual void OnInput(EVL_MinigameInput Input) override;

	// Length in REAL seconds.
	UPROPERTY(EditAnywhere, Category = "Timing Lanes", meta = (ClampMin = "5.0"))
	float Duration = 20.f;

	UPROPERTY(EditAnywhere, Category = "Timing Lanes", meta = (ClampMin = "1"))
	int32 PieceCount = 12;

	// Seconds a piece takes to fall from the top to the catch line.
	UPROPERTY(EditAnywhere, Category = "Timing Lanes", meta = (ClampMin = "0.3"))
	float FallTime = 1.2f;

	// A press within this many seconds of the perfect moment counts as a catch.
	UPROPERTY(EditAnywhere, Category = "Timing Lanes", meta = (ClampMin = "0.05"))
	float HitWindow = 0.2f;

	// Resource pieces collected per caught piece.
	UPROPERTY(EditAnywhere, Category = "Timing Lanes", meta = (ClampMin = "1"))
	int32 PiecesPerCatch = 1;

private:
	TArray<FVL_TimingPiece> Pieces;
	int32 HitCount = 0;
};