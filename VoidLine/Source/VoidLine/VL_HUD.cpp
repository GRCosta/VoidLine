#include "VL_HUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "VL_EquipmentDefinition.h"
#include "VL_LoadoutSubsystem.h"
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
		DrawBed();
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

void AVL_HUD::DrawBed()
{
	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	const UVL_LoadoutSubsystem* Loadout = GameInstance ? GameInstance->GetSubsystem<UVL_LoadoutSubsystem>() : nullptr;
	if (!Loadout)
	{
		return;
	}

	const FVL_BedLayout& Bed = Loadout->GetLayout();

	// Layout: bottom-left corner.
	const float CellSize = 40.f;
	const float Gap = 3.f;
	const float Height = Bed.GridSize.Y * (CellSize + Gap);
	const float X0 = 40.f;
	const float Y0 = Canvas->SizeY - Height - 40.f;

	UFont* Font = GEngine->GetMediumFont();
	DrawText(FString::Printf(TEXT("BED %dx%d"), Bed.GridSize.X, Bed.GridSize.Y),
		FLinearColor::White, X0, Y0 - 28.f, Font, 1.f);

	// Empty cells.
	for (int32 Y = 0; Y < Bed.GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < Bed.GridSize.X; ++X)
		{
			DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.8f),
				X0 + X * (CellSize + Gap), Y0 + Y * (CellSize + Gap), CellSize, CellSize);
		}
	}

	// Placed items, with a short label on their first cell.
	for (const FVL_PlacedItem& Item : Bed.Items)
	{
		FLinearColor Color(0.2f, 0.7f, 0.8f); // container
		FString Label;

		if (Item.Equipment)
		{
			Color = Item.Equipment->Color;
			Label = Item.Equipment->DisplayName.ToString().Left(2);
		}
		else if (Item.ContainerFor)
		{
			Label = Item.ContainerFor->DisplayName.ToString().Left(2);
		}

		const TArray<FIntPoint> Cells = FVL_BedLayout::GetItemCells(Item);
		for (int32 i = 0; i < Cells.Num(); ++i)
		{
			const float CellX = X0 + Cells[i].X * (CellSize + Gap);
			const float CellY = Y0 + Cells[i].Y * (CellSize + Gap);
			DrawRect(Color, CellX, CellY, CellSize, CellSize);

			if (i == 0)
			{
				DrawText(Label, FLinearColor::Black, CellX + 6.f, CellY + 8.f, Font, 1.f);
			}
		}
	}
}