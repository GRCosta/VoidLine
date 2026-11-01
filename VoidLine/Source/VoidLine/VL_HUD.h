#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VL_HUD.generated.h"

// Draws the collection minigame. Placeholder look, to be replaced by real UI later.
UCLASS()
class VOIDLINE_API AVL_HUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};