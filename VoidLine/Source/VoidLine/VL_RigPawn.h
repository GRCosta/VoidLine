#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VL_RigPawn.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UStaticMeshComponent;
class UVL_HealthComponent;
class UVL_ExposureComponent;
struct FEnhancedInputActionValueBinding;
struct FInputActionValue;

class UVL_WeaponComponent;
class UVL_EnemySpawnerComponent;

UCLASS()
class VOIDLINE_API AVL_RigPawn : public ACharacter
{
	GENERATED_BODY()

public:
	AVL_RigPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

	// Debug console commands
	UFUNCTION(Exec)
	void VL_Sleep();

	UFUNCTION(Exec)
	void VL_SetNight(int32 NewNight);

protected:
	virtual void BeginPlay() override;

	void ApplyMovementTuning();

	// ---- Components ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VL|Components")
	TObjectPtr<UStaticMeshComponent> RigMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VL|Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VL|Components")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VL|Components")
	TObjectPtr<UVL_WeaponComponent> WeaponComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VL|Components")
	TObjectPtr<UVL_HealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VL|Components")
	TObjectPtr<UVL_ExposureComponent> ExposureComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VL|Components")
	TObjectPtr<UVL_EnemySpawnerComponent> EnemySpawner;

	// ---- Input assets (assign these in the Blueprint subclass) ----
	UPROPERTY(EditDefaultsOnly, Category = "VL|Input")
	TObjectPtr<UInputMappingContext> DrivingContext;

	UPROPERTY(EditDefaultsOnly, Category = "VL|Input")
	TObjectPtr<UInputAction> SteerAction;     // Axis1D

	UPROPERTY(EditDefaultsOnly, Category = "VL|Input")
	TObjectPtr<UInputAction> ThrottleAction;  // Axis1D (right trigger)

	UPROPERTY(EditDefaultsOnly, Category = "VL|Input")
	TObjectPtr<UInputAction> BrakeAction;     // Axis1D (left trigger)

	UPROPERTY(EditDefaultsOnly, Category = "VL|Input")
	TObjectPtr<UInputAction> LookAction;      // Axis2D (right stick)

	UPROPERTY(EditDefaultsOnly, Category = "VL|Input")
	TObjectPtr<UInputAction> DriftAction;     // Digital (bool)

	// ---- Driving tuning (cm, cm/s, degrees) ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float MaxSpeed = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float Acceleration = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float CoastDeceleration = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float ReverseSpeedScale = 0.4f;

	// How fast the steering value follows the stick. Lower = heavier feel.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float SteerResponse = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float TurnRateLowSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float TurnRateHighSpeed = 70.f;

	// Below this speed the rig can't turn on the spot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float MinSpeedForFullTurn = 300.f;

	// Higher grip = velocity follows heading faster. Lower = more slide.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float GripNormal = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float GripDrift = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Driving")
	float DriftTurnMultiplier = 1.5f;

	// ---- Camera tuning ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Camera")
	float CameraArmLength = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Camera")
	float CameraPitch = -20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Camera")
	float CameraLookYawRange = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Camera")
	float CameraLookPitchRange = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Camera")
	float CameraLookInterpSpeed = 5.f;

	UFUNCTION()
	void HandleRigDeath();

private:

	void OnSteer(const FInputActionValue& Value);
	void OnThrottle(const FInputActionValue& Value);
	void OnBrake(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnDrift(const FInputActionValue& Value);

	float SteerInput = 0.f;
	float ThrottleInput = 0.f;
	float BrakeInput = 0.f;
	FVector2D LookInput = FVector2D::ZeroVector;
	bool bDriftInput = false;

	float SmoothedSteer = 0.f;
	FRotator CurrentCameraRot = FRotator::ZeroRotator;
};