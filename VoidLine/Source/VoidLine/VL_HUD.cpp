#include "VL_HUD.h"

#include "Engine/Canvas.h"
#include "GameFramework/Pawn.h"
#include "VL_MinigameComponent.h"

void AVL_HUD::DrawHUD()
{
	Super::DrawHUD();

	const APawn* OwnerPawn = GetOwningPawn();
	if (!Canvas || !OwnerPawn)
	{
		return;
	}

	const UVL_MinigameComponent* Minigame = OwnerPawn->FindComponentByClass<UVL_MinigameComponent>();
	if (Minigame && Minigame->IsRunning())
	{
		Minigame->GetCurrent()->Draw(this, Canvas->SizeX, Canvas->SizeY);
	}
}