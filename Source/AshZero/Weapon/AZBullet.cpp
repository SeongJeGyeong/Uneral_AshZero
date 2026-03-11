#include "Weapon/AZBullet.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AshZero.h"

AAZBullet::AAZBullet()
{
    PrimaryActorTick.bCanEverTick = false;

    // 충돌
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    CollisionComp->SetSphereRadius(5.f);
    CollisionComp->SetCollisionProfileName(TEXT("BlockAll"));
    CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    CollisionComp->OnComponentHit.AddDynamic(this, &AAZBullet::OnHit);
    RootComponent = CollisionComp;

    // 메시
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetRelativeScale3D(FVector(0.1f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        MeshComp->SetStaticMesh(SphereMesh.Object);
    }

    // Projectile Movement
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 5000.f;
    ProjectileMovement->MaxSpeed = 5000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.f;

    // 5초 후 자동 파괴
    InitialLifeSpan = 5.f;
}

void AAZBullet::BeginPlay()
{
    Super::BeginPlay();
}

void AAZBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != this && OtherActor != GetInstigator())
    {
        PRINT_LOG(TEXT("Bullet Hit: %s"), *OtherActor->GetName());

        UGameplayStatics::ApplyDamage(
            OtherActor,
            Damage,
            GetInstigatorController(),
            this,
            UDamageType::StaticClass()
        );

        Destroy();
    }
}