#include "VL_RigPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "VL_ExposureComponent.h"
#include "VL_HealthComponent.h"
#include "VL_WeaponComponent.h"
#include "VL_WorldStateSubsystem.h"


AVL_RigPawn::AVL_RigPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// We steer manually, so the controller must not rotate the rig.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Squat-ish collision. Adjust to match your rig mesh later.
	GetCapsuleComponent()->InitCapsuleSize(110.f, 110.f);

	RigMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RigMesh"));
	RigMesh->SetupAttachment(GetCapsuleComponent());
	RigMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = CameraArmLength;
	CameraBoom->SetRelativeRotation(FRotator(CameraPitch, 0.f, 0.f));
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 8.f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 5.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<UVL_HealthComponent>(TEXT("HealthComponent"));
	ExposureComponent = CreateDefaultSubobject<UVL_ExposureComponent>(TEXT("ExposureComponent"));
	WeaponComponent = CreateDefaultSubobject<UVL_WeaponComponent>(TEXT("WeaponComponent"));
}

void AVL_RigPawn::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDeath.AddDynamic(this, &AVL_RigPawn::HandleRigDeath);

	ApplyMovementTuning();

	CameraBoom->TargetArmLength = CameraArmLength;
	CurrentCameraRot = FRotator(CameraPitch, 0.f, 0.f);
	CameraBoom->SetRelativeRotation(CurrentCameraRot);
}

// Re-applies the editable tuning values to the movement component,
// so changes made in the Blueprint subclass take effect.
void AVL_RigPawn::ApplyMovementTuning()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = false;
	Move->bUseControllerDesiredRotation = false;
	Move->MaxWalkSpeed = MaxSpeed;
	Move->MaxAcceleration = Acceleration;
	Move->BrakingDecelerationWalking = CoastDeceleration;
	Move->bUseSeparateBrakingFriction = true;
}

void AVL_RigPawn::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DrivingContext)
				{
					Subsystem->AddMappingContext(DrivingContext, 0);
				}
			}
		}
	}
}

void AVL_RigPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Error, TEXT("VL_RigPawn: input component is NOT an EnhancedInputComponent"));
		return;
	}

	auto Bind = [this, EIC](const UInputAction* Action,
		void (AVL_RigPawn::* Handler)(const FInputActionValue&), const TCHAR* Name)
		{
			if (!Action)
			{
				UE_LOG(LogTemp, Error, TEXT("VL_RigPawn: %s is not assigned in the Blueprint"), Name);
				return;
			}
			EIC->BindAction(Action, ETriggerEvent::Triggered, this, Handler);
			EIC->BindAction(Action, ETriggerEvent::Completed, this, Handler);
			EIC->BindAction(Action, ETriggerEvent::Canceled, this, Handler);
		};

	Bind(SteerAction, &AVL_RigPawn::OnSteer, TEXT("SteerAction"));
	Bind(ThrottleAction, &AVL_RigPawn::OnThrottle, TEXT("ThrottleAction"));
	Bind(BrakeAction, &AVL_RigPawn::OnBrake, TEXT("BrakeAction"));
	Bind(LookAction, &AVL_RigPawn::OnLook, TEXT("LookAction"));
	Bind(DriftAction, &AVL_RigPawn::OnDrift, TEXT("DriftAction"));
}

void AVL_RigPawn::OnSteer(const FInputActionValue& Value) { SteerInput = Value.Get<float>(); }
void AVL_RigPawn::OnThrottle(const FInputActionValue& Value) { ThrottleInput = Value.Get<float>(); }
void AVL_RigPawn::OnBrake(const FInputActionValue& Value) { BrakeInput = Value.Get<float>(); }
void AVL_RigPawn::OnLook(const FInputActionValue& Value) { LookInput = Value.Get<FVector2D>(); }
void AVL_RigPawn::OnDrift(const FInputActionValue& Value) { bDriftInput = Value.Get<bool>(); }

void AVL_RigPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// ---- Read input ----
	const float Steer = SteerInput;
	const float Throttle = ThrottleInput;
	const float Brake = BrakeInput;
	const FVector2D Look = LookInput;
	const bool bDrifting = bDriftInput;

	const FVector Forward = GetActorForwardVector();
	const float ForwardSpeed = FVector::DotProduct(GetVelocity(), Forward);
	const float Speed = GetVelocity().Size2D();
	const float SpeedAlpha = FMath::Clamp(Speed / MaxSpeed, 0.f, 1.f);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(101, 0.f, FColor::Green,
			FString::Printf(TEXT("Steer %.2f  Thr %.2f  Brk %.2f  Look %.2f,%.2f  Drift %d  Speed %.0f"),
				Steer, Throttle, Brake, Look.X, Look.Y, bDrifting ? 1 : 0, Speed));
	}

	// ---- Steering: smoothed, speed-sensitive turn rate ----
	SmoothedSteer = FMath::FInterpTo(SmoothedSteer, Steer, DeltaSeconds, SteerResponse);

	const float TurnRate = FMath::Lerp(TurnRateLowSpeed, TurnRateHighSpeed, SpeedAlpha)
		* (bDrifting ? DriftTurnMultiplier : 1.f);
	const float LowSpeedScale = FMath::Clamp(Speed / MinSpeedForFullTurn, 0.f, 1.f);
	const float ReverseSign = (ForwardSpeed < -50.f) ? -1.f : 1.f; // steer like a car when reversing

	AddActorWorldRotation(FRotator(0.f,
		SmoothedSteer * TurnRate * LowSpeedScale * ReverseSign * DeltaSeconds, 0.f));

	// ---- Throttle / brake / reverse ----
	// Brake pulls the rig down and, if held, into reverse (scaled slower).
	const float Drive = Throttle - Brake;
	if (!FMath::IsNearlyZero(Drive, 0.01f))
	{
		AddMovementInput(GetActorForwardVector(), Drive > 0.f ? Drive : Drive * ReverseSpeedScale);
	}

	// ---- Grip: lower friction lets the rig slide through turns ----
	GetCharacterMovement()->GroundFriction = bDrifting ? GripDrift : GripNormal;

	// ---- Camera: right stick offsets the view, springs back to center ----
	const FRotator TargetCamRot(
		CameraPitch + Look.Y * CameraLookPitchRange,
		Look.X * CameraLookYawRange,
		0.f);
	CurrentCameraRot = FMath::RInterpTo(CurrentCameraRot, TargetCamRot, DeltaSeconds, CameraLookInterpSpeed);
	CameraBoom->SetRelativeRotation(CurrentCameraRot);

}
void AVL_RigPawn::HandleRigDeath()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(104, 10.f, FColor::Red, TEXT("RIG DESTROYED - run failed (placeholder)"));
	}

	ExposureComponent->StopRun();
	WeaponComponent->SetWeapon(nullptr);

	// Zero the stored input so the rig stops driving, then stop listening to the controller.
	SteerInput = ThrottleInput = BrakeInput = 0.f;
	LookInput = FVector2D::ZeroVector;
	bDriftInput = false;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
}

void AVL_RigPawn::VL_Sleep()
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UVL_WorldStateSubsystem* State = GI->GetSubsystem<UVL_WorldStateSubsystem>())
		{
			State->AdvanceNight();
		}
	}
}

void AVL_RigPawn::VL_SetNight(int32 NewNight)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UVL_WorldStateSubsystem* State = GI->GetSubsystem<UVL_WorldStateSubsystem>())
		{
			State->SetNight(NewNight);
		}
	}
}