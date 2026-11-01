#include "VL_HealthComponent.h"

#include "GameFramework/Actor.h"

UVL_HealthComponent::UVL_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVL_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	bDead = false;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UVL_HealthComponent::HandleTakeAnyDamage);
	}
}

void UVL_HealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	ApplyDamage(Damage);
}

void UVL_HealthComponent::ApplyDamage(float Amount)
{
	if (bDead || Amount <= 0.f)
	{
		return;
	}

	const float Old = Health;
	Health = FMath::Clamp(Health - Amount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(Health, MaxHealth, Health - Old);

	if (Health <= 0.f)
	{
		bDead = true;
		OnDeath.Broadcast();
	}
}