// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AZInteractionComponent.h"
#include "Interface//AZInteractable.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AZDataAsset.h"
#include "Character/AZPlayerCharacter.h"
#include "AZGameplayTags.h"
#include "AshZero.h"

// Sets default values for this component's properties
UAZInteractionComponent::UAZInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UAZInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAZInteractionComponent::SetupInputBinding(UEnhancedInputComponent* PlayerInput)
{
	if (!PlayerInput || !OwnerCharacter || !OwnerCharacter->InputDataAsset)
	{
		PRINT_LOG(TEXT("SetupInputBinding Failed!"));
		return;
	}
	UAZDataAsset* InputData = OwnerCharacter->InputDataAsset;
	if (InputData == nullptr)
		return;
	PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Interact), ETriggerEvent::Started, this, &UAZInteractionComponent::PrimaryInteract);
}




// Called every frame
void UAZInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FindInteractable();
}

void UAZInteractionComponent::FindInteractable()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
		return;

	FVector EyeLocation;
	FRotator EyeRotation;

	Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FVector End = EyeLocation + (EyeRotation.Vector() * TraceDistance);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	FHitResult Hit;
	bool bHit = GetWorld()->SweepSingleByObjectType(
		Hit,
		EyeLocation,
		End,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	AActor* HitActor = bHit ? Hit.GetActor() : nullptr;
	UObject* NewFocusedObject = nullptr;
	AActor* NewFocusedActor = nullptr;

	if (HitActor)
	{
		
		if (HitActor->Implements<UAZInteractable>())
		{
			NewFocusedObject = HitActor;
			NewFocusedActor = HitActor;
		}
		else
		{
			UActorComponent* FoundComp = HitActor->FindComponentByInterface(UAZInteractable::StaticClass());
			if (FoundComp)
			{
				NewFocusedObject = FoundComp;
				NewFocusedActor = HitActor;
			}
		}
	}

	if (FocusedObject != NewFocusedObject)
	{
		if (FocusedObject)
		{
			OnInteractableLost.Broadcast();
		}
		FocusedObject = NewFocusedObject;
		FocusedActorCached = NewFocusedActor;

		if (FocusedObject)
		{
			OnInteractableFound.Broadcast(FocusedActorCached);
		}

	
	}
	 //if (bHit) DrawDebugSphere(GetWorld(), Hit.Location, TraceRadius, 12, FColor::Green, false, 0.0f);
	 //else DrawDebugLine(GetWorld(), EyeLocation, End, FColor::Red, false, 0.0f);
}

void UAZInteractionComponent::PrimaryInteract()
{
	if (FocusedObject)
	{
		if (FocusedObject->GetClass()->ImplementsInterface(UAZInteractable::StaticClass()))
		{
			IAZInteractable::Execute_Interact(FocusedObject, GetOwner());
		}

		if (AActor* TargetActor = Cast<AActor>(FocusedObject))
		{
			TArray<UActorComponent*> InteractableComps = TargetActor->GetComponentsByInterface(UAZInteractable::StaticClass());

			for (UActorComponent* Comp : InteractableComps)
			{
				IAZInteractable::Execute_Interact(Comp, GetOwner());
			}
		}
	}
}
