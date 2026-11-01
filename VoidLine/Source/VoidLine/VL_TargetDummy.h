#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VL_TargetDummy.generated.h"

class UStaticMeshComponent;
class UVL_HealthComponent;

// Stationary test target. Tagged VL_Enemy so weapons can find it.
UCLASS()
class VOIDLINE_API AVL_TargetDummy : public AActor
{
	GENERATED_BODY()

public:
	AVL_TargetDummy();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth, float Delta);

	UFUNCTION()
	void HandleDeath();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UVL_HealthComponent> Health;
};