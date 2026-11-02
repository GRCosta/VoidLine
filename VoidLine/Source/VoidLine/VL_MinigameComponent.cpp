#include "VL_MinigameComponent.h"

#include "GameFramework/Actor.h"
#include "VL_ExposureComponent.h"

UVL_MinigameComponent::UVL_MinigameComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVL_MinigameComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		ExposureSource = Owner->FindComponentByClass<UVL_ExposureComponent>();
	}
}

void UVL_MinigameComponent::StartMinigame(const UVL_MinigameBase* Template)
{
	if (!Template)
	{
		return;
	}

	// Copy the template so one run's state never leaks into the data asset.
	Current = DuplicateObject<UVL_MinigameBase>(Template, this);
	Current->Start();
}

void UVL_MinigameComponent::CancelMinigame()
{
	Current = nullptr;
	OnFinished.Unbind();
}

void UVL_MinigameComponent::SendInput(EVL_MinigameInput Input)
{
	if (IsRunning())
	{
		Current->SendInput(Input);
	}
}

void UVL_MinigameComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsRunning())
	{
		return;
	}

	// Minigames can react to Exposure (no effects yet, but the value is always available).
	if (ExposureSource)
	{
		Current->SetExposure(ExposureSource->GetExposure01());
	}

	Current->Update();

	if (Current->IsFinished())
	{
		const int32 Pieces = Current->GetCollectedPieces();

		// Unbind first, so the listener can safely start or cancel things in its callback.
		FVL_OnMinigameFinished Callback = OnFinished;
		OnFinished.Unbind();
		Callback.ExecuteIfBound(Pieces);
	}
}