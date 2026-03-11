// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AZThrowProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"

// Sets default values
AAZThrowProjectile::AAZThrowProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(10.0f);
	CollisionComp->SetCollisionProfileName(TEXT("NoCollision"));
	CollisionComp->SetSimulatePhysics(false);

	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));

	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;

	ProjectileMovement->ProjectileGravityScale = 1.0f;

	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->Bounciness = 0.3f; // Æ¨±â´Â Á¤µµ (0~1)
	ProjectileMovement->Friction = 0.6f;   // ¸¶Âû·Â

	ProjectileMovement->bRotationFollowsVelocity = false;

	ProjectileMovement->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void AAZThrowProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("Grenade BeginPlay Executed! Authority: %s"), HasAuthority() ? TEXT("Server") : TEXT("Client"));

	if (HasAuthority() && CollisionComp)
	{
		CollisionComp->OnComponentHit.AddDynamic(this, &AAZThrowProjectile::OnHit);
		UE_LOG(LogTemp, Warning, TEXT("Grenade OnHit Binding Complete"));
	}

	if (ProjectileMovement)
	{
		float CurrentSpeed = ProjectileMovement->Velocity.Size();
		if (CurrentSpeed <= 0.1f) CurrentSpeed = ProjectileMovement->InitialSpeed;
		float SpinSpeed = CurrentSpeed * 0.5f;
		TumblingRotationRate = FRotator(-SpinSpeed, 0.0f, 0.0f);
	}
}

void AAZThrowProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Grenade OnHit! HitActor: %s"), *GetNameSafe(OtherActor));
	if (HasAuthority())
	{
		if (OtherActor && OtherActor != this && OtherActor != GetOwner())
		{
			Explode_Server(Hit.ImpactPoint);
		}
	}
}

void AAZThrowProjectile::Explode_Server_Implementation(FVector ImpactLocation)
{
	UGameplayStatics::ApplyRadialDamage(
		this,
		ExplosionDamage,
		ImpactLocation,
		ExplosionRadius,
		UDamageType::StaticClass(),
		TArray<AActor*>(),
		this,
		GetInstigatorController(),
		true
	);

	ExplodeFX_Multicast(ImpactLocation);

	if (MeshComp)
	{
		MeshComp->SetVisibility(false);
	}

	if (CollisionComp)
	{
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
	}

	
	if (bShowDrawDebug)
	{
		DrawDebugSphere(GetWorld(), ImpactLocation, ExplosionRadius, 32, FColor::Magenta, false, 2.0f);
	}
	

	SetLifeSpan(0.4f);
}


void AAZThrowProjectile::ExplodeFX_Multicast_Implementation(FVector Location)
{
	if (ExplosionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionVFX,
			Location,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
	}

	PlaySFX_Multicast(ESFXType::Greande_Boom);
}

void AAZThrowProjectile::PlaySFX_Multicast_Implementation(ESFXType SFXType)
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr) return;
	UAZSoundManagerSubsystem* SoundManager = GI->GetSubsystem<UAZSoundManagerSubsystem>();
	if (SoundManager == nullptr) return;

	SoundManager->PlaySFX(this, SFXType);
}

// Called every frame
void AAZThrowProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (!bIsThrowing) return;

	if (MeshComp)
	{
		MeshComp->AddLocalRotation(TumblingRotationRate * DeltaTime);
	}
}

void AAZThrowProjectile::SetVelocity(FVector Direction)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
	}
}

void AAZThrowProjectile::InitializeProjectile(float InDamage)
{
	ExplosionDamage = InDamage;
}

void AAZThrowProjectile::AddIgnoreActor(AActor* ActorToIgnore)
{
	if (CollisionComp && ActorToIgnore)
	{
		CollisionComp->IgnoreActorWhenMoving(ActorToIgnore, true);
	}
}

void AAZThrowProjectile::SetHeldState()
{
	if (CollisionComp)
	{
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	PlaySFX_Multicast(ESFXType::Grenade_Pin);
	bIsThrowing = false;
}

void AAZThrowProjectile::LaunchProjectile(FVector LaunchDirection)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetReplicateMovement(true);

	if (CollisionComp)
	{
		CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = LaunchDirection * ProjectileMovement->InitialSpeed;
		ProjectileMovement->SetUpdatedComponent(CollisionComp);
		ProjectileMovement->Activate();
		bIsThrowing = true;
	}
}

