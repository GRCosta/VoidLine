#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VL_BedLayout.h"
#include "VL_LoadoutPreset.generated.h"

// A ready-made bed arrangement. Used for testing now; the garage will use presets
// for "last loadout" and for each city's recommended setups.
UCLASS(BlueprintType)
class VOIDLINE_API UVL_LoadoutPreset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	FIntPoint GridSize = FIntPoint(4, 4);

	// Equipment pieces set Equipment. Containers set Container For and leave Equipment empty.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TArray<FVL_PlacedItem> Items;
};