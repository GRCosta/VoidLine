#pragma once

#include "CoreMinimal.h"
#include "VL_MinigameBase.h"
#include "VL_ChipperMinigame.generated.h"

struct FVL_ChipperLog
{
	int32 Lane = 0;
	float HitTime = 0.f; // seconds since the minigame started
	uint8 State = 0;     // 0 = falling, 1 = chipped, 2 = missed
};

// Logs fall down several lanes. LB / RB slide the chipper one lane left / right.
// A log is chipped into wood pieces if the chipper is under it when it lands.
UCLASS(meta = (DisplayName = "Wood Chipper"))
class VOIDLINE_API UVL_ChipperMinigame : public UVL_MinigameBase
{
	GENERATED_BODY()

public:
	virtual void Draw(AHUD* HUD, float CanvasWidth, float CanvasHeight) const override;

protected:
	virtual void OnStart() override;
	virtual void OnUpdate(float Elapsed) override;
	virtual void OnInput(EVL_MinigameInput Input) override;

	// Length in REAL seconds.
	UPROPERTY(EditAnywhere, Category = "Wood Chipper", meta = (ClampMin = "5.0"))
	float Duration = 20.f;

	UPROPERTY(EditAnywhere, Category = "Wood Chipper", meta = (ClampMin = "2", ClampMax = "5"))
	int32 LaneCount = 3;

	// Logs in the schedule. Fewer are used if they can't all fit in the duration.
	UPROPERTY(EditAnywhere, Category = "Wood Chipper", meta = (ClampMin = "1"))
	int32 LogCount = 14;

	// Seconds a log takes to fall from the top to the chipper.
	UPROPERTY(EditAnywhere, Category = "Wood Chipper", meta = (ClampMin = "0.5"))
	float FallTime = 1.6f;

	// How fast the chipper slides, in lanes per second.
	UPROPERTY(EditAnywhere, Category = "Wood Chipper", meta = (ClampMin = "1.0"))
	float ChipperSpeed = 6.f;

	// A log is caught if the chipper is within this many lanes of it when it lands.
	UPROPERTY(EditAnywhere, Category = "Wood Chipper", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float CatchTolerance = 0.5f;

	// Wood pieces produced per caught log.
	UPROPERTY(EditAnywhere, Category = "Wood Chipper", meta = (ClampMin = "1"))
	int32 PiecesPerLog = 4;

private:
	TArray<FVL_ChipperLog> Logs;
	int32 HitCount = 0;
	int32 TargetLane = 1;
	float ChipperPos = 1.f;
	float LastElapsed = 0.f;
};