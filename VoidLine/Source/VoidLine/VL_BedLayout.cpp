#include "VL_BedLayout.h"

#include "VL_EquipmentDefinition.h"
#include "VL_ResourceDefinition.h"

TArray<FIntPoint> FVL_BedLayout::GetItemCells(const FVL_PlacedItem& Item)
{
	TArray<FIntPoint> Result;

	if (Item.Equipment)
	{
		const int32 Turns = ((Item.Rotation % 4) + 4) % 4;

		for (const FIntPoint& Cell : Item.Equipment->Cells)
		{
			FIntPoint Rotated = Cell;
			for (int32 Turn = 0; Turn < Turns; ++Turn)
			{
				Rotated = FIntPoint(-Rotated.Y, Rotated.X);
			}
			Result.Add(Item.Origin + Rotated);
		}
	}
	else if (Item.ContainerFor)
	{
		Result.Add(Item.Origin);
	}

	return Result;
}

int32 FVL_BedLayout::FindItemAtCell(FIntPoint Cell) const
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (GetItemCells(Items[i]).Contains(Cell))
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool FVL_BedLayout::CanPlace(const FVL_PlacedItem& Item) const
{
	const TArray<FIntPoint> Cells = GetItemCells(Item);
	if (Cells.Num() == 0)
	{
		return false;
	}

	for (const FIntPoint& Cell : Cells)
	{
		const bool bOutside = Cell.X < 0 || Cell.Y < 0 || Cell.X >= GridSize.X || Cell.Y >= GridSize.Y;
		if (bOutside || FindItemAtCell(Cell) != INDEX_NONE)
		{
			return false;
		}
	}

	// One container per resource type.
	if (Item.ContainerFor && HasContainerFor(Item.ContainerFor))
	{
		return false;
	}

	return true;
}

bool FVL_BedLayout::TryPlace(const FVL_PlacedItem& Item)
{
	if (!CanPlace(Item))
	{
		return false;
	}
	Items.Add(Item);
	return true;
}

bool FVL_BedLayout::RemoveAtCell(FIntPoint Cell)
{
	const int32 Index = FindItemAtCell(Cell);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	Items.RemoveAt(Index);
	return true;
}

bool FVL_BedLayout::HasContainerFor(const UVL_ResourceDefinition* Resource) const
{
	for (const FVL_PlacedItem& Item : Items)
	{
		if (Item.ContainerFor.Get() == Resource)
		{
			return true;
		}
	}
	return false;
}

bool FVL_BedLayout::HasCollectorFor(const UVL_ResourceDefinition* Resource) const
{
	for (const FVL_PlacedItem& Item : Items)
	{
		if (Item.Equipment
			&& Item.Equipment->Type == EVL_EquipmentType::Collector
			&& Item.Equipment->CollectsResource.Get() == Resource)
		{
			return true;
		}
	}
	return false;
}