#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VL_TargetDummy.generated.h"

class UStaticMeshComponent;

// Stationary test target. Tagged VL_Enemy so weapons can find it.
UCLASS()
class VOIDLINE_API AVL_TargetDummy : public AActor
{
	GENERATED_BODY()

public:
	AVL_TargetDummy();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "VL|Dummy")
	float MaxHealth = 100.f;

private:
	float Health = 100.f;
};