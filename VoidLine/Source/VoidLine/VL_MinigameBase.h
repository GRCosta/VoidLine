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
// Create a subclass per minigame; resource data assets pick which one to use.
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class VOIDLINE_API UVL_MinigameBase : public UObject
{
	GENERATED_BODY()

public:
	void Start();
	void Update();
	void SendInput(EVL_MinigameInput Input);

	// Draws the minigame on the HUD. The canvas size is passed in because the HUD's canvas is protected.
	virtual void Draw(AHUD* HUD, float CanvasWidth, float CanvasHeight) const {}

	bool IsFinished() const { return bFinished; }
	bool WasSuccessful() const { return bSuccess; }
	int32 GetHits() const { return Hits; }
	int32 GetTotal() const { return Total; }

	// Real seconds since Start.
	float GetElapsed() const;

protected:
	virtual void OnStart() {}
	virtual void OnUpdate(float Elapsed) {}
	virtual void OnInput(EVL_MinigameInput Input) {}

	// Call when the minigame is over.
	void Complete(bool bInSuccess, int32 InHits, int32 InTotal);

private:
	double StartTime = 0.0;
	bool bStarted = false;
	bool bFinished = false;
	bool bSuccess = false;
	int32 Hits = 0;
	int32 Total = 0;
};