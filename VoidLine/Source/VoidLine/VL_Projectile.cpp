#include "VL_Projectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AVL_Projectile::AVL_Projectile()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(25.f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	// Projectiles are PhysicsBody objects that ignore each other, but still hit everything else.
	Collision->SetCollisionObjectType(ECC_PhysicsBody);
	Collision->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Collision->SetNotifyRigidBodyCollision(true); // lets OnComponentHit fire
	Collision->OnComponentHit.AddDynamic(this, &AVL_Projectile::OnHit);
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.4f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->ProjectileGravityScale = 0.f;
	Movement->bRotationFollowsVelocity = true;
}

void AVL_Projectile::Init(float InDamage, float InSpeed, float InRange)
{
	Damage = InDamage;

	// Never collide with the rig that fired us.
	if (AActor* Shooter = GetOwner())
	{
		Collision->IgnoreActorWhenMoving(Shooter, true);
	}

	Movement->MaxSpeed = InSpeed;
	Movement->Velocity = GetActorForwardVector() * InSpeed;

	// Despawn after travelling roughly Range. Follows world time, like the movement.
	SetLifeSpan(InRange / FMath::Max(InSpeed, 1.f));
}

void AVL_Projectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
	}
	Destroy();
}