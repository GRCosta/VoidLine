#include "VL_ExposureComponent.h"

#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "VL_HealthComponent.h"

UVL_ExposureComponent::UVL_ExposureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVL_ExposureComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		Health = Owner->FindComponentByClass<UVL_HealthComponent>();
	}
	if (bAutoStart)
	{
		StartRun();
	}
}

void UVL_ExposureComponent::StartRun()
{
	Elapsed = 0.f;
	SetStage(EVL_ExposureStage::Calm);
	bRunning = true;
}

void UVL_ExposureComponent::StopRun()
{
	bRunning = false;
}

float UVL_ExposureComponent::GetExposure01() const
{
	return FMath::Clamp(Elapsed / RunDuration, 0.f, 1.f);
}

float UVL_ExposureComponent::GetSecondsRemaining() const
{
	return FMath::Max(RunDuration - Elapsed, 0.f);
}

void UVL_ExposureComponent::SetStage(EVL_ExposureStage NewStage)
{
	Stage = NewStage;
	OnStageChanged.Broadcast(Stage);
}

void UVL_ExposureComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bRunning)
	{
		return;
	}

	// DeltaTime is already scaled by world time dilation.
	Elapsed += DeltaTime;

	const float Exposure = GetExposure01();
	EVL_ExposureStage NewStage = EVL_ExposureStage::Calm;
	if (Exposure >= CriticalThreshold)
	{
		NewStage = EVL_ExposureStage::Critical;
	}
	else if (Exposure >= UneasyThreshold)
	{
		NewStage = EVL_ExposureStage::Uneasy;
	}
	if (NewStage != Stage)
	{
		SetStage(NewStage);
	}

	if (Stage == EVL_ExposureStage::Critical && Health && !Health->IsDead())
	{
		Health->ApplyDamage(CriticalDamagePerSecond * DeltaTime);
	}

	if (bShowDebugText && GEngine)
	{
		const TCHAR* StageName = TEXT("Calm");
		FColor Color = FColor::Green;
		if (Stage == EVL_ExposureStage::Uneasy) { StageName = TEXT("Uneasy");   Color = FColor::Yellow; }
		if (Stage == EVL_ExposureStage::Critical) { StageName = TEXT("CRITICAL"); Color = FColor::Red; }

		const int32 Sec = FMath::CeilToInt(GetSecondsRemaining());
		GEngine->AddOnScreenDebugMessage(102, 0.f, Color,
			FString::Printf(TEXT("Run %02d:%02d left | Exposure %.0f%% | %s | Rig HP %.0f"),
				Sec / 60, Sec % 60, Exposure * 100.f, StageName, Health ? Health->GetHealth() : 0.f));
	}
}