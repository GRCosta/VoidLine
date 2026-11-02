#pragma once

#include "CoreMinimal.h"
#include "VL_BedLayout.generated.h"

class UVL_EquipmentDefinition;
class UVL_ResourceDefinition;

// One thing placed in the bed: either a piece of equipment or a 1x1 resource container.
USTRUCT(BlueprintType)
struct FVL_PlacedItem
{
	GENERATED_BODY()

	// Set for equipment pieces.
	UPROPERTY()
	TObjectPtr<UVL_EquipmentDefinition> Equipment = nullptr;

	// Set for 1x1 resource containers (Equipment is null then).
	UPROPERTY()
	TObjectPtr<UVL_ResourceDefinition> ContainerFor = nullptr;

	UPROPERTY()
	FIntPoint Origin = FIntPoint::ZeroValue;

	// Quarter turns, 0 to 3.
	UPROPERTY()
	int32 Rotation = 0;
};

// The rig's bed: a grid with placement rules. Pure data, so the garage screen can edit it later.
USTRUCT(BlueprintType)
struct VOIDLINE_API FVL_BedLayout
{
	GENERATED_BODY()

	UPROPERTY()
	FIntPoint GridSize = FIntPoint(4, 4);

	UPROPERTY()
	TArray<FVL_PlacedItem> Items;

	// The grid cells an item covers, after rotation.
	static TArray<FIntPoint> GetItemCells(const FVL_PlacedItem& Item);

	bool CanPlace(const FVL_PlacedItem& Item) const;
	bool TryPlace(const FVL_PlacedItem& Item);

	// Index of the item covering this cell, or INDEX_NONE.
	int32 FindItemAtCell(FIntPoint Cell) const;

	// Removes whichever item covers this cell. Returns true if something was removed.
	bool RemoveAtCell(FIntPoint Cell);

	bool HasContainerFor(const UVL_ResourceDefinition* Resource) const;
	bool HasCollectorFor(const UVL_ResourceDefinition* Resource) const;
};