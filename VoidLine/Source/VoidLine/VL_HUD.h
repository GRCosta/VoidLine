#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VL_HUD.generated.h"

class UVL_InventoryComponent;

// Draws the cargo readout and the collection minigame. Placeholder look, to be replaced by real UI later.
UCLASS()
class VOIDLINE_API AVL_HUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawInventory(const UVL_InventoryComponent* Inventory);
};