#include "VL_InteractionComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "VL_InventoryComponent.h"
#include "VL_MinigameComponent.h"
#include "VL_ResourceDefinition.h"
#include "VL_ResourceNode.h"

UVL_InteractionComponent::UVL_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVL_InteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bCollecting && GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
		bCollecting = false;
	}
	Super::EndPlay(EndPlayReason);
}

void UVL_InteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetWorld() || !GetOwner())
	{
		return;
	}

	if (bCollecting)
	{
		// The node vanished mid-minigame: give up.
		if (!Active.IsValid())
		{
			FinishCollection(false, 0);
		}
		return;
	}

	// Look for a node in reach a few times per second.
	FocusScanTimer -= DeltaTime;
	if (FocusScanTimer <= 0.f)
	{
		FocusScanTimer = 0.1f;
		FindFocus();
	}
}

void UVL_InteractionComponent::FindFocus()
{
	const FVector Origin = GetOwner()->GetActorLocation();
	float BestDistSq = FMath::Square(InteractRadius);
	AVL_ResourceNode* Best = nullptr;

	for (TActorIterator<AVL_ResourceNode> It(GetWorld()); It; ++It)
	{
		const float DistSq = FVector::DistSquared(Origin, It->GetActorLocation());
		if (DistSq <= BestDistSq)
		{
			BestDistSq = DistSq;
			Best = *It;
		}
	}
	SetFocus(Best);
}

void UVL_InteractionComponent::SetFocus(AVL_ResourceNode* NewFocus)
{
	AVL_ResourceNode* Old = Focus.Get();
	if (Old == NewFocus)
	{
		return;
	}
	if (Old)
	{
		Old->SetFocused(false);
	}
	Focus = NewFocus;
	if (NewFocus)
	{
		NewFocus->SetFocused(true);
	}
}

void UVL_InteractionComponent::TryInteract()
{
	AVL_ResourceNode* Node = Focus.Get();
	if (!GetWorld() || bCollecting || !Node)
	{
		return;
	}

	const UVL_ResourceDefinition* Definition = Node->GetDefinition();
	const UVL_MinigameBase* Template = Definition ? Definition->Minigame.Get() : nullptr;
	if (!Template)
	{
		UE_LOG(LogTemp, Error, TEXT("VL_InteractionComponent: resource '%s' has no minigame assigned"), *Node->GetDisplayName());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(108, 4.f, FColor::Red,
				FString::Printf(TEXT("%s has no minigame assigned in its data asset"), *Node->GetDisplayName()));
		}
		return;
	}

	UVL_MinigameComponent* Minigame = GetOwner()->FindComponentByClass<UVL_MinigameComponent>();
	if (!Minigame)
	{
		UE_LOG(LogTemp, Error, TEXT("VL_InteractionComponent: owner has no MinigameComponent"));
		return;
	}

	bCollecting = true;
	Active = Node;
	UGameplayStatics::SetGlobalTimeDilation(this, CollectTimeDilation);

	Minigame->OnFinished.BindUObject(this, &UVL_InteractionComponent::HandleMinigameFinished);
	Minigame->StartMinigame(Template);
}

void UVL_InteractionComponent::CancelCollection()
{
	if (bCollecting)
	{
		FinishCollection(false, 0);
	}
}

void UVL_InteractionComponent::HandleMinigameFinished(int32 PiecesCollected)
{
	FinishCollection(true, PiecesCollected);
}

void UVL_InteractionComponent::FinishCollection(bool bCompleted, int32 PiecesCollected)
{
	if (UVL_MinigameComponent* Minigame = GetOwner()->FindComponentByClass<UVL_MinigameComponent>())
	{
		Minigame->CancelMinigame(); // no effect if it already finished
	}

	bCollecting = false;
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);

	AVL_ResourceNode* Node = Active.Get();
	Active.Reset();

	if (GEngine && Node)
	{
		const FString NodeName = Node->GetDisplayName();
		if (bCompleted)
		{
			GEngine->AddOnScreenDebugMessage(107, 3.f, FColor::Green,
				FString::Printf(TEXT("Collected %d %s"), PiecesCollected, *NodeName));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(107, 3.f, FColor::Orange,
				FString::Printf(TEXT("Collection of %s interrupted"), *NodeName));
		}
	}

	// A finished session always consumes the node, whatever the amount collected.
	if (bCompleted && Node)
	{
		if (UVL_InventoryComponent* Inventory = GetOwner()->FindComponentByClass<UVL_InventoryComponent>())
		{
			Inventory->AddResource(Node->GetDefinition(), PiecesCollected);
		}

		Focus.Reset();
		Node->Destroy();
	}
}