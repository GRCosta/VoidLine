#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VL_Projectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class VOIDLINE_API AVL_Projectile : public AActor
{
	GENERATED_BODY()

public:
	AVL_Projectile();

	// Called by the weapon right after spawning.
	void Init(float InDamage, float InSpeed, float InRange);

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> Movement;

private:
	float Damage = 10.f;
};