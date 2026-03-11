// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(InputComp)) return;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!IsValid(PlayerController)) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IsValid(InputSubsystem)) return;

	InputSubsystem->ClearAllMappings();
	InputSubsystem->AddMappingContext(IMC, 0);

	if (IA_Move) InputComp->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
	if (IA_Look) InputComp->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
	//if (IA_Jump) InputComp->BindAction(IA_Jump, ETriggerEvent::Started, this, &AMyCharacter::Jump);
}

void AMyCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MoveVal = Value.Get<FVector2D>();

	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	if (MoveVal.X != 0.0f)
	{
		const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(Direction, MoveVal.X);
	}

	if (MoveVal.Y != 0.0f)
	{
		const FVector Direction = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(Direction, MoveVal.Y);
	}
}


void AMyCharacter::Look(const FInputActionValue& Value)
{
	FVector2D RotateVal = Value.Get<FVector2D>();

	if (RotateVal.X != 0.0f)
	{
		AddControllerYawInput(RotateVal.X);
	}

	if (RotateVal.Y != 0.0f)
	{
		AddControllerPitchInput(-RotateVal.Y);
	}
}

//void AMyCharacter::Jump(const FInputActionValue& Value)
//{
//	Super::Jump();
//}

