// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/AZInteractComponent.h"
#include "AZNonPlayerCharacter.h"

UAZInteractComponent::UAZInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAZInteractComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UAZInteractComponent::Interact_Implementation(AActor* InstigatorActor)
{
}

