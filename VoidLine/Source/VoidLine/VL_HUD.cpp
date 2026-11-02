#include "VL_HUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "VL_InventoryComponent.h"
#include "VL_MinigameComponent.h"
#include "VL_ResourceDefinition.h"
#include "VL_RunContract.h"

void AVL_HUD::DrawHUD()
{
	Super::DrawHUD();

	const APawn* OwnerPawn = GetOwningPawn();
	if (!Canvas || !OwnerPawn)
	{
		return;
	}

	if (const UVL_InventoryComponent* Inventory = OwnerPawn->FindComponentByClass<UVL_InventoryComponent>())
	{
		DrawInventory(Inventory);
	}

	const UVL_MinigameComponent* Minigame = OwnerPawn->FindComponentByClass<UVL_MinigameComponent>();
	if (Minigame && Minigame->IsRunning())
	{
		Minigame->GetCurrent()->Draw(this, Canvas->SizeX, Canvas->SizeY);
	}
}

void AVL_HUD::DrawInventory(const UVL_InventoryComponent* Inventory)
{
	UFont* Font = GEngine->GetMediumFont();

	// Layout: top-left corner.
	const float X = 40.f;
	float Y = 40.f;
	const float BarW = 180.f;

	const UVL_RunContract* Contract = Inventory->GetContract();
	const FString Title = (Contract && !Contract->Title.IsEmpty()) ? Contract->Title.ToString() : TEXT("CARGO");
	DrawText(Title, FLinearColor::White, X, Y, Font, 1.1f);
	Y += 34.f;

	TArray<const UVL_ResourceDefinition*> Shown;

	// Resources the contract asks for: have / need, with a progress bar.
	if (Contract)
	{
		for (const FVL_ResourceAmount& Need : Contract->Needs)
		{
			if (!Need.Resource)
			{
				continue;
			}

			const int32 Have = Inventory->GetCount(Need.Resource.Get());
			const float Ratio = (Need.Amount > 0)
				? FMath::Clamp(static_cast<float>(Have) / Need.Amount, 0.f, 1.f)
				: 1.f;

			FLinearColor Color = FLinearColor::White;
			if (Have >= Need.Amount)
			{
				Color = FLinearColor(0.3f, 1.f, 0.4f);   // done
			}
			else if (Have > 0)
			{
				Color = FLinearColor(1.f, 0.85f, 0.4f);  // in progress
			}

			DrawText(FString::Printf(TEXT("%s  %d / %d"), *Need.Resource->DisplayName.ToString(), Have, Need.Amount),
				Color, X, Y, Font, 1.f);
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), X, Y + 24.f, BarW, 8.f);
			DrawRect(Color, X, Y + 24.f, BarW * Ratio, 8.f);

			Y += 46.f;
			Shown.Add(Need.Resource.Get());
		}
	}

	// Anything collected that the contract doesn't ask for.
	for (const FVL_ResourceAmount& Item : Inventory->GetItems())
	{
		if (!Item.Resource || Item.Amount <= 0 || Shown.Contains(Item.Resource.Get()))
		{
			continue;
		}

		DrawText(FString::Printf(TEXT("%s  %d"), *Item.Resource->DisplayName.ToString(), Item.Amount),
			FLinearColor(0.7f, 0.7f, 0.7f), X, Y, Font, 1.f);
		Y += 30.f;
	}
}