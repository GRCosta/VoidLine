#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VL_MinigameBase.generated.h"

class AHUD;

UENUM(BlueprintType)
enum class EVL_MinigameInput : uint8
{
	Left,   // left bumper
	Right   // right bumper
};

// Base for every collection minigame. Runs on REAL time, so world slow-motion doesn't stretch it.
// A minigame has no fail state: it runs its timer and reports how many pieces were collected.
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class VOIDLINE_API UVL_MinigameBase : public UObject
{
	GENERATED_BODY()

public:
	void Start();
	void Update();
	void SendInput(EVL_MinigameInput Input);

	// Current Exposure (0 to 1), pushed in every frame. Minigames can use it to get harder or stranger.
	void SetExposure(float InExposure01) { ExposureValue = InExposure01; }

	// Draws the minigame on the HUD. The canvas size is passed in because the HUD's canvas is protected.
	virtual void Draw(AHUD* HUD, float CanvasWidth, float CanvasHeight) const {}

	bool IsFinished() const { return bFinished; }

	// Resource pieces collected. Valid once finished.
	int32 GetCollectedPieces() const { return CollectedPieces; }

	// Real seconds since Start.
	float GetElapsed() const;

protected:
	virtual void OnStart() {}
	virtual void OnUpdate(float Elapsed) {}
	virtual void OnInput(EVL_MinigameInput Input) {}

	// Call when the minigame is over.
	void Complete(int32 InCollectedPieces);

	float GetExposure() const { return ExposureValue; }

private:
	double StartTime = 0.0;
	bool bStarted = false;
	bool bFinished = false;
	int32 CollectedPieces = 0;
	float ExposureValue = 0.f;
};