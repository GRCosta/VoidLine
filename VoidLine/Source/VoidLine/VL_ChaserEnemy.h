#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VL_ChaserEnemy.generated.h"

class UStaticMeshComponent;
class UVL_HealthComponent;

// Simple swarm enemy: walks straight at the player's rig and hurts it on contact.
UCLASS()
class VOIDLINE_API AVL_ChaserEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AVL_ChaserEnemy();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleDeath();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UVL_HealthComponent> Health;

	UPROPERTY(EditAnywhere, Category = "VL|Enemy")
	float MaxHealth = 10.f;

	UPROPERTY(EditAnywhere, Category = "VL|Enemy")
	float MoveSpeed = 600.f;

	// Center-to-center distance (cm) at which it can hurt the rig.
	UPROPERTY(EditAnywhere, Category = "VL|Enemy")
	float ContactRange = 190.f;

	UPROPERTY(EditAnywhere, Category = "VL|Enemy")
	float ContactDamage = 3.f;

	UPROPERTY(EditAnywhere, Category = "VL|Enemy")
	float ContactCooldown = 1.f;

private:
	TWeakObjectPtr<AActor> Target;
	float NextAttackTime = 0.f;
};