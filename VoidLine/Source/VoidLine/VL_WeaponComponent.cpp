#include "VL_WeaponComponent.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "VL_Projectile.h"
#include "VL_WeaponDefinition.h"

UVL_WeaponComponent::UVL_WeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVL_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	RestartTimer();
}

void UVL_WeaponComponent::SetWeapon(UVL_WeaponDefinition* NewWeapon)
{
	Weapon = NewWeapon;
	RestartTimer();
}

void UVL_WeaponComponent::RestartTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(FireTimer);

	if (Weapon && Weapon->FireInterval > 0.f)
	{
		// World timers follow global time dilation, so slow-motion minigames slow the weapon too.
		World->GetTimerManager().SetTimer(FireTimer, this, &UVL_WeaponComponent::Fire, Weapon->FireInterval, true);
	}
}

AActor* UVL_WeaponComponent::FindNearestTarget() const
{
	if (!Weapon || !GetOwner())
	{
		return nullptr;
	}

	TArray<AActor*> Candidates;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, Candidates);

	const FVector Origin = GetOwner()->GetActorLocation();
	float BestDistSq = FMath::Square(Weapon->Range);
	AActor* Best = nullptr;

	for (AActor* Candidate : Candidates)
	{
		if (!IsValid(Candidate))
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
		if (DistSq <= BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Candidate;
		}
	}
	return Best;
}

void UVL_WeaponComponent::Fire()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Weapon || !Weapon->ProjectileClass)
	{
		return;
	}

	AActor* Target = FindNearestTarget();
	if (!Target)
	{
		return;
	}

	const FVector Muzzle = Owner->GetActorTransform().TransformPosition(MuzzleOffset);
	const FVector Direction = (Target->GetActorLocation() - Muzzle).GetSafeNormal();

	FActorSpawnParameters Params;
	Params.Owner = Owner;
	Params.Instigator = Cast<APawn>(Owner);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AVL_Projectile* Projectile = GetWorld()->SpawnActor<AVL_Projectile>(
		Weapon->ProjectileClass, Muzzle, Direction.Rotation(), Params))
	{
		Projectile->Init(Weapon->Damage, Weapon->ProjectileSpeed, Weapon->Range);
	}
}