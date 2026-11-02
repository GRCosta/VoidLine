#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VL_BedLayout.h"
#include "VL_LoadoutSubsystem.generated.h"

class UVL_EquipmentDefinition;
class UVL_ResourceDefinition;

DECLARE_MULTICAST_DELEGATE(FVL_OnLoadoutChanged);

// What the rig carries into a run. Lives for the whole session, so the city hub can
// edit it and the driving map can read it.
UCLASS()
class VOIDLINE_API UVL_LoadoutSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool PlaceEquipment(UVL_EquipmentDefinition* Equipment, FIntPoint Origin, int32 Rotation);
	bool PlaceContainer(UVL_ResourceDefinition* Resource, FIntPoint Cell);
	bool RemoveAtCell(FIntPoint Cell);
	void ClearBed();

	// Changes the grid size. Clears the bed.
	void SetBedSize(FIntPoint NewSize);

	const FVL_BedLayout& GetLayout() const { return BedLayout; }

	FVL_OnLoadoutChanged OnChanged;

private:
	UPROPERTY()
	FVL_BedLayout BedLayout;
};