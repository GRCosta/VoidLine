#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VL_EquipmentDefinition.generated.h"

class UVL_ResourceDefinition;
class UVL_WeaponDefinition;

UENUM(BlueprintType)
enum class EVL_EquipmentType : uint8
{
	Collector,
	Weapon,
	Other
};

// A piece of equipment that takes up space in the rig's bed. One data asset per piece.
UCLASS(BlueprintType)
class VOIDLINE_API UVL_EquipmentDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	EVL_EquipmentType Type = EVL_EquipmentType::Collector;

	// The cells the piece covers: X = column, Y = row, relative to the piece's origin.
	// Example, an L piece: (0,0) (0,1) (0,2) (1,2).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TArray<FIntPoint> Cells = { FIntPoint(0, 0) };

	// Collectors only: the resource this piece lets the rig collect.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UVL_ResourceDefinition> CollectsResource;

	// Weapons only: the weapon this piece provides.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UVL_WeaponDefinition> Weapon;

	// Color of the piece in the bed display.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FLinearColor Color = FLinearColor(0.8f, 0.5f, 0.2f);
};