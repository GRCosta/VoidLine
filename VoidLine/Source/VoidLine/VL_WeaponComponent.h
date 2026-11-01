#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VL_WeaponComponent.generated.h"

class UVL_WeaponDefinition;

// Auto-attack: fires at the nearest tagged target in range on a repeating timer.
UCLASS(ClassGroup = (VL), meta = (BlueprintSpawnableComponent))
class VOIDLINE_API UVL_WeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVL_WeaponComponent();

	UFUNCTION(BlueprintCallable, Category = "VL|Weapon")
	void SetWeapon(UVL_WeaponDefinition* NewWeapon);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Weapon")
	TObjectPtr<UVL_WeaponDefinition> Weapon;

	// Actors with this tag count as targets.
	UPROPERTY(EditAnywhere, Category = "VL|Weapon")
	FName TargetTag = TEXT("VL_Enemy");

	// Where projectiles spawn, in the owner's local space.
	UPROPERTY(EditAnywhere, Category = "VL|Weapon")
	FVector MuzzleOffset = FVector(0.f, 0.f, 120.f);

private:
	void RestartTimer();
	void Fire();
	AActor* FindNearestTarget() const;

	FTimerHandle FireTimer;
};