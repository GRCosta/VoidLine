#include "VL_WeaponComponent.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "VL_Projectile.h"
#include "VL_WeaponDefinition.h"

namespace
{
	// Direction to fire so a projectile of the given speed meets a moving target.
	FVector ComputeLeadDirection(const FVector& Shooter, const FVector& TargetPos,
		const FVector& TargetVel, float Speed)
	{
		const FVector D = TargetPos - Shooter;
		const float A = FVector::DotProduct(TargetVel, TargetVel) - Speed * Speed;
		const float B = 2.f * FVector::DotProduct(D, TargetVel);
		const float C = FVector::DotProduct(D, D);

		float T = -1.f;
		if (FMath::Abs(A) < KINDA_SMALL_NUMBER)
		{
			if (!FMath::IsNearlyZero(B))
			{
				T = -C / B;
			}
		}
		else
		{
			const float Disc = B * B - 4.f * A * C;
			if (Disc >= 0.f)
			{
				const float Root = FMath::Sqrt(Disc);
				const float T1 = (-B - Root) / (2.f * A);
				const float T2 = (-B + Root) / (2.f * A);
				T = (T1 > 0.f && T2 > 0.f) ? FMath::Min(T1, T2) : FMath::Max(T1, T2);
			}
		}

		if (T <= 0.f)
		{
			return D.GetSafeNormal(); // no solution, fall back to aiming straight at it
		}
		return (D + TargetVel * T).GetSafeNormal();
	}
}


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
	const FVector Direction = ComputeLeadDirection(Muzzle, Target->GetActorLocation(),
		Target->GetVelocity(), Weapon->ProjectileSpeed);

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