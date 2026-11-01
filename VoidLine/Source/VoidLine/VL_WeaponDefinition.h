#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VL_WeaponDefinition.generated.h"

class AVL_Projectile;

// Stats for one weapon. Create instances in the Content Browser and tune them there.
UCLASS(BlueprintType)
class VOIDLINE_API UVL_WeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AVL_Projectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float Damage = 10.f;

	// Seconds between shots.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.05"))
	float FireInterval = 0.6f;

	// Max distance (cm) to pick a target and how far a projectile can travel.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float Range = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 3000.f;
};