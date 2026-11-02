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
#include "VL_EnemySpawnerComponent.h"
#include "VL_ExposureComponent.h"
#include "VL_HealthComponent.h"
#include "VL_InteractionComponent.h"
#include "VL_InventoryComponent.h"
#include "VL_MinigameComponent.h"
#include "VL_WeaponComponent.h"
#include "VL_WorldStateSubsystem.h"
#include "VL_EquipmentDefinition.h"
#include "VL_LoadoutSubsystem.h"
#include "VL_LoadoutPreset.h"
#include "VL_ResourceDefinition.h"
#include "VL_WeaponDefinition.h"


namespace
{
	UVL_LoadoutSubsystem* FindLoadout(const UObject* WorldContext)
	{
		if (const UWorld* World = WorldContext->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				return GameInstance->GetSubsystem<UVL_LoadoutSubsystem>();
			}
		}
		return nullptr;
	}

	void ShowMessage(const FString& Text, const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(109, 4.f, Color, Text);
		}
	}
}

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
	EnemySpawner = CreateDefaultSubobject<UVL_EnemySpawnerComponent>(TEXT("EnemySpawner"));
	WeaponComponent = CreateDefaultSubobject<UVL_WeaponComponent>(TEXT("WeaponComponent"));
	InteractionComponent = CreateDefaultSubobject<UVL_InteractionComponent>(TEXT("InteractionComponent"));
	MinigameComponent = CreateDefaultSubobject<UVL_MinigameComponent>(TEXT("MinigameComponent"));
	InventoryComponent = CreateDefaultSubobject<UVL_InventoryComponent>(TEXT("InventoryComponent"));
}

void AVL_RigPawn::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDeath.AddDynamic(this, &AVL_RigPawn::HandleRigDeath);

	ApplyMovementTuning();

	CameraBoom->TargetArmLength = CameraArmLength;
	CurrentCameraRot = FRotator(CameraPitch, 0.f, 0.f);
	CameraBoom->SetRelativeRotation(CurrentCameraRot);

	if (UVL_LoadoutSubsystem* Loadout = FindLoadout(this))
	{
		// Test setup: fill the bed from a preset when it's empty.
		if (TestLoadout && Loadout->GetLayout().Items.Num() == 0)
		{
			Loadout->ApplyPreset(TestLoadout);
		}

		// Re-read the bed whenever it changes (console commands for now, the garage later).
		Loadout->OnChanged.AddUObject(this, &AVL_RigPawn::ApplyLoadout);
	}
	ApplyLoadout();
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

	if (InteractAction)
	{
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AVL_RigPawn::OnInteract);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VL_RigPawn: InteractAction is not assigned in the Blueprint"));
	}
	
	if (LaneLeftAction)
	{
		EIC->BindAction(LaneLeftAction, ETriggerEvent::Started, this, &AVL_RigPawn::OnLaneLeft);
	}
	if (LaneRightAction)
	{
		EIC->BindAction(LaneRightAction, ETriggerEvent::Started, this, &AVL_RigPawn::OnLaneRight);
	}
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
	// While a collection minigame runs, the rig holds still (but the camera still looks around).
	const bool bLocked = InteractionComponent && InteractionComponent->IsCollecting();
	const float Steer = bLocked ? 0.f : SteerInput;
	const float Throttle = bLocked ? 0.f : ThrottleInput;
	const float Brake = bLocked ? 0.f : BrakeInput;
	const FVector2D Look = LookInput;
	const bool bDrifting = bLocked ? false : bDriftInput;

	if (bLocked)
	{
		GetCharacterMovement()->StopMovementImmediately();
	}

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

	// ---- Damping: remove sideways slide and let the rig slow down when coasting ----
	{
		UCharacterMovementComponent* Move = GetCharacterMovement();
		if (Move->IsMovingOnGround())
		{
			const FVector Fwd = GetActorForwardVector();
			const FVector Right = GetActorRightVector();
			FVector Vel = Move->Velocity;

			float ForwardSpeedNow = FVector::DotProduct(Vel, Fwd);
			float Lateral = FVector::DotProduct(Vel, Right);

			// Sideways velocity decays exponentially; less so while drifting.
			const float Grip = bDrifting ? DriftLateralGrip : LateralGrip;
			Lateral *= FMath::Exp(-Grip * DeltaSeconds);

			// Forward velocity decays when off the throttle, faster with the brake.
			float Drag = 0.f;
			if (Throttle < 0.05f)
			{
				Drag += CoastDrag;
			}
			if (Brake > 0.05f)
			{
				Drag += BrakeDrag * Brake;
			}
			ForwardSpeedNow *= FMath::Exp(-Drag * DeltaSeconds);

			const float VelZ = Vel.Z;
			Vel = Fwd * ForwardSpeedNow + Right * Lateral;
			Vel.Z = VelZ;
			Move->Velocity = Vel;
		}
	}

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
	EnemySpawner->StopSpawning();
	InteractionComponent->CancelCollection();
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

void AVL_RigPawn::OnInteract(const FInputActionValue& Value)
{
	InteractionComponent->TryInteract();
}

void AVL_RigPawn::OnLaneLeft(const FInputActionValue& Value)
{
	MinigameComponent->SendInput(EVL_MinigameInput::Left);
}

void AVL_RigPawn::OnLaneRight(const FInputActionValue& Value)
{
	MinigameComponent->SendInput(EVL_MinigameInput::Right);
}

void AVL_RigPawn::VL_Equip(int32 EquipmentIndex, int32 X, int32 Y, int32 Rotation)
{
	UVL_LoadoutSubsystem* Loadout = FindLoadout(this);
	if (!Loadout || !DebugEquipment.IsValidIndex(EquipmentIndex) || !DebugEquipment[EquipmentIndex])
	{
		ShowMessage(TEXT("VL_Equip: bad equipment index (fill Debug Equipment on BP_VL_Rig)"), FColor::Red);
		return;
	}

	UVL_EquipmentDefinition* Equipment = DebugEquipment[EquipmentIndex];
	const FString Name = Equipment->DisplayName.ToString();

	if (Loadout->PlaceEquipment(Equipment, FIntPoint(X, Y), Rotation))
	{
		ShowMessage(FString::Printf(TEXT("Placed %s at (%d,%d), rotation %d"), *Name, X, Y, Rotation), FColor::Green);
	}
	else
	{
		ShowMessage(FString::Printf(TEXT("Cannot place %s there (outside the bed or overlapping)"), *Name), FColor::Orange);
	}
}

void AVL_RigPawn::VL_Container(int32 ResourceIndex, int32 X, int32 Y)
{
	UVL_LoadoutSubsystem* Loadout = FindLoadout(this);
	if (!Loadout || !DebugResources.IsValidIndex(ResourceIndex) || !DebugResources[ResourceIndex])
	{
		ShowMessage(TEXT("VL_Container: bad resource index (fill Debug Resources on BP_VL_Rig)"), FColor::Red);
		return;
	}

	UVL_ResourceDefinition* Resource = DebugResources[ResourceIndex];
	const FString Name = Resource->DisplayName.ToString();

	if (Loadout->PlaceContainer(Resource, FIntPoint(X, Y)))
	{
		ShowMessage(FString::Printf(TEXT("Placed %s container at (%d,%d)"), *Name, X, Y), FColor::Green);
	}
	else
	{
		ShowMessage(FString::Printf(TEXT("Cannot place %s container there (outside, overlapping, or already have one)"), *Name), FColor::Orange);
	}
}

void AVL_RigPawn::VL_Unequip(int32 X, int32 Y)
{
	if (UVL_LoadoutSubsystem* Loadout = FindLoadout(this))
	{
		if (Loadout->RemoveAtCell(FIntPoint(X, Y)))
		{
			ShowMessage(FString::Printf(TEXT("Removed the item at (%d,%d)"), X, Y), FColor::Green);
		}
		else
		{
			ShowMessage(FString::Printf(TEXT("Nothing at (%d,%d)"), X, Y), FColor::Orange);
		}
	}
}

void AVL_RigPawn::VL_ClearBed()
{
	if (UVL_LoadoutSubsystem* Loadout = FindLoadout(this))
	{
		Loadout->ClearBed();
		ShowMessage(TEXT("Bed cleared"), FColor::Green);
	}
}

void AVL_RigPawn::VL_BedSize(int32 Width, int32 Height)
{
	if (UVL_LoadoutSubsystem* Loadout = FindLoadout(this))
	{
		Loadout->SetBedSize(FIntPoint(Width, Height));
		ShowMessage(FString::Printf(TEXT("Bed resized to %dx%d (cleared)"), Width, Height), FColor::Green);
	}
}

void AVL_RigPawn::ApplyLoadout()
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	UVL_WeaponDefinition* Chosen = nullptr;

	if (const UVL_LoadoutSubsystem* Loadout = FindLoadout(this))
	{
		for (const FVL_PlacedItem& Item : Loadout->GetLayout().Items)
		{
			if (Item.Equipment && Item.Equipment->Type == EVL_EquipmentType::Weapon && Item.Equipment->Weapon)
			{
				Chosen = Item.Equipment->Weapon;
				break;
			}
		}
	}

	// No weapon in the bed means an unarmed rig. Only the first weapon is used for now.
	WeaponComponent->SetWeapon(Chosen);
}