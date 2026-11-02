#include "VL_LoadoutSubsystem.h"

#include "VL_EquipmentDefinition.h"
#include "VL_ResourceDefinition.h"
#include "VL_LoadoutPreset.h"


bool UVL_LoadoutSubsystem::PlaceEquipment(UVL_EquipmentDefinition* Equipment, FIntPoint Origin, int32 Rotation)
{
	if (!Equipment)
	{
		return false;
	}

	FVL_PlacedItem Item;
	Item.Equipment = Equipment;
	Item.Origin = Origin;
	Item.Rotation = Rotation;

	if (!BedLayout.TryPlace(Item))
	{
		return false;
	}
	OnChanged.Broadcast();
	return true;
}

bool UVL_LoadoutSubsystem::PlaceContainer(UVL_ResourceDefinition* Resource, FIntPoint Cell)
{
	if (!Resource)
	{
		return false;
	}

	FVL_PlacedItem Item;
	Item.ContainerFor = Resource;
	Item.Origin = Cell;

	if (!BedLayout.TryPlace(Item))
	{
		return false;
	}
	OnChanged.Broadcast();
	return true;
}

bool UVL_LoadoutSubsystem::RemoveAtCell(FIntPoint Cell)
{
	if (!BedLayout.RemoveAtCell(Cell))
	{
		return false;
	}
	OnChanged.Broadcast();
	return true;
}

void UVL_LoadoutSubsystem::ClearBed()
{
	BedLayout.Items.Reset();
	OnChanged.Broadcast();
}

void UVL_LoadoutSubsystem::SetBedSize(FIntPoint NewSize)
{
	BedLayout.GridSize = FIntPoint(FMath::Clamp(NewSize.X, 2, 10), FMath::Clamp(NewSize.Y, 2, 10));
	BedLayout.Items.Reset();
	OnChanged.Broadcast();
}

void UVL_LoadoutSubsystem::ApplyPreset(const UVL_LoadoutPreset* Preset)
{
	if (!Preset)
	{
		return;
	}

	BedLayout.GridSize = FIntPoint(FMath::Clamp(Preset->GridSize.X, 2, 10), FMath::Clamp(Preset->GridSize.Y, 2, 10));
	BedLayout.Items.Reset();

	for (const FVL_PlacedItem& Item : Preset->Items)
	{
		BedLayout.TryPlace(Item);
	}

	OnChanged.Broadcast();
}

bool UVL_LoadoutSubsystem::CanCollect(const UVL_ResourceDefinition* Resource) const
{
	return BedLayout.HasCollectorFor(Resource) && BedLayout.HasContainerFor(Resource);
}