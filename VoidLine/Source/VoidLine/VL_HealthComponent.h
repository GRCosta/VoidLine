#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VL_HealthComponent.generated.h"

class UDamageType;
class AController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVL_OnHealthChanged, float, NewHealth, float, MaxHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVL_OnDeath);

// Shared health for the rig and enemies. Listens to normal Unreal damage events.
UCLASS(ClassGroup = (VL), meta = (BlueprintSpawnableComponent))
class VOIDLINE_API UVL_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVL_HealthComponent();

	// Direct damage that bypasses the Unreal damage system (used by Exposure).
	UFUNCTION(BlueprintCallable, Category = "VL|Health")
	void ApplyDamage(float Amount);

	UFUNCTION(BlueprintPure, Category = "VL|Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "VL|Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "VL|Health")
	bool IsDead() const { return bDead; }

	UPROPERTY(BlueprintAssignable, Category = "VL|Health")
	FVL_OnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "VL|Health")
	FVL_OnDeath OnDeath;

	// Sets max health and refills to full. Call before BeginPlay.
	void SetMaxHealth(float NewMax);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

private:
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);

	float Health = 100.f;
	bool bDead = false;
};