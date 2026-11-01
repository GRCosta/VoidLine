#include "VL_ChaserEnemy.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "VL_HealthComponent.h"

AVL_ChaserEnemy::AVL_ChaserEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::Disabled;
	bUseControllerRotationYaw = false;

	GetCapsuleComponent()->InitCapsuleSize(45.f, 90.f);
	// Enemies don't block the rig or each other; the rig drives through them.
	// Projectiles still hit them because those use a different channel.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bRunPhysicsWithNoController = true; // no AI controller needed
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 540.f, 0.f);
	Move->MaxWalkSpeed = MoveSpeed;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(ConeMesh.Object);
	}

	Health = CreateDefaultSubobject<UVL_HealthComponent>(TEXT("Health"));

	Tags.Add(TEXT("VL_Enemy"));
}

void AVL_ChaserEnemy::BeginPlay()
{
	// Set max health before the health component's own BeginPlay resets current health.
	Health->SetMaxHealth(MaxHealth);

	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	Health->OnDeath.AddDynamic(this, &AVL_ChaserEnemy::HandleDeath);
	Target = UGameplayStatics::GetPlayerPawn(this, 0);
}

void AVL_ChaserEnemy::HandleDeath()
{
	Destroy();
}

void AVL_ChaserEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AActor* Goal = Target.Get();
	if (!Goal)
	{
		Target = UGameplayStatics::GetPlayerPawn(this, 0);
		return;
	}

	FVector ToTarget = Goal->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;
	const float Distance = ToTarget.Size();

	if (Distance > KINDA_SMALL_NUMBER)
	{
		AddMovementInput(ToTarget / Distance, 1.f);
	}

	if (Distance <= ContactRange)
	{
		// World time follows time dilation, so slow-motion slows enemy attacks too.
		const float Now = GetWorld()->GetTimeSeconds();
		if (Now >= NextAttackTime)
		{
			NextAttackTime = Now + ContactCooldown;
			UGameplayStatics::ApplyDamage(Goal, ContactDamage, nullptr, this, UDamageType::StaticClass());
		}
	}
}