// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/AZNonPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "System/Player/AZPlayerController.h"
#include "System/Player/Components/AZDialogUIComponent.h"
#include "Camera/CameraComponent.h"
#include "Util/AZDefine.h"

// Sets default values
AAZNonPlayerCharacter::AAZNonPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->SetCollisionProfileName(TEXT("Pawn"));
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);

	HighlightColor = EHighlightColor::White;
	InteractionText = FText::FromString(TEXT("대화"));
}

// Called when the game starts or when spawned
void AAZNonPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AAZNonPlayerCharacter::Interact_Implementation(AActor* InstigatorActor)
{
	if (AAZPlayerController* PlayerController = InstigatorActor->GetInstigatorController<AAZPlayerController>())
	{
		OnInteraction.Broadcast(this);

		PlayerController->DialogUIComp->OpenDialogBox(NameTag, EnableButtons);
		PlayerController->ChangeCameraViewTarget(this);
		InstigatorActor->SetActorHiddenInGame(true);
	}
}