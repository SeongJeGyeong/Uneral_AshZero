// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/AZWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Components/DecalComponent.h"
#include "Weapon/AZBullet.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/AZHealthComponent.h"
#include "Interface/AZStatusInterface.h"
#include "Net/UnrealNetwork.h"
#include "AshZero.h"

AAZWeapon::AAZWeapon()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);
    bAlwaysRelevant = true;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 기본 메시 (BP에서 오버라이드)
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> DefaultMesh(
        TEXT("/Script/Engine.SkeletalMesh'/Game/Assets/MilitaryWeapSilver/Weapons/Pistols_A.Pistols_A'"));

    if (DefaultMesh.Succeeded())
    {
        WeaponMesh->SetSkeletalMesh(DefaultMesh.Object);
    }

    // 기본 레벨 데이터 (BP에서 오버라이드)
    FWeaponLevelData DefaultLevel;
    DefaultLevel.Damage = 10.0f;
    DefaultLevel.MaxAmmo = 30;
    DefaultLevel.FireRate = 5.0f;
    DefaultLevel.ReloadTime = 2.0f;
    DefaultLevel.MaxRange = 10000.0f;
    LevelDataArray.Add(DefaultLevel);
}

void AAZWeapon::BeginPlay()
{
    Super::BeginPlay();

    // 현재 레벨 데이터 적용
    ApplyLevelData();
}

void AAZWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAZWeapon, CurrentAmmo);
    DOREPLIFETIME(AAZWeapon, CurrentLevel);
}

// ==================== 레벨 시스템 ====================

void AAZWeapon::SetLevel(int32 NewLevel)
{
    if (LevelDataArray.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] LevelDataArray is empty!"), *GetName());
        return;
    }

    CurrentLevel = FMath::Clamp(NewLevel, 0, LevelDataArray.Num() - 1);
    ApplyLevelData();

    UE_LOG(LogTemp, Log, TEXT("[%s] Set Level: %d (Damage: %.1f, FireRate: %.1f, MaxAmmo: %d)"),
        *GetName(), CurrentLevel, Damage, 1.0f / FireTime, MaxAmmo);
}

void AAZWeapon::OnRep_CurrentLevel()
{
    // 클라이언트에서 레벨 변경 시 스탯 재적용
    ApplyLevelData();
}

void AAZWeapon::ApplyLevelData()
{
    const FWeaponLevelData& Data = GetCurrentLevelData();

    Damage = Data.Damage;
    MaxAmmo = Data.MaxAmmo;
    FireTime = Data.GetFireTime();
    ReloadTime = Data.ReloadTime;
    MaxDistance = Data.MaxRange;

    // BeginPlay 시점에만 탄약 초기화
    if (!HasActorBegunPlay() || CurrentAmmo == 0)
    {
        CurrentAmmo = MaxAmmo;
    }

    // 효과 로그
    UE_LOG(LogTemp, Log, TEXT("[%s] Level %d Applied - Effects: %d"), *GetName(), CurrentLevel, Data.Effects.Num());
}

FWeaponLevelData AAZWeapon::GetCurrentLevelData() const
{
    if (LevelDataArray.IsValidIndex(CurrentLevel))
    {
        return LevelDataArray[CurrentLevel];
    }
    return FWeaponLevelData();
}

bool AAZWeapon::HasEffect(EWeaponEffect EffectType) const
{
    return GetCurrentLevelData().HasEffect(EffectType);
}

FWeaponEffectData AAZWeapon::GetEffectData(EWeaponEffect EffectType) const
{
    return GetCurrentLevelData().GetEffectData(EffectType);
}

// ==================== 발사 ====================

void AAZWeapon::Fire(FVector StartPos, FVector Direction)
{
    if (CurrentAmmo <= 0)
    {
        return;
    }

    CurrentAmmo--;
    /*
    // 멀티샷 처리
    if (HasEffect(EWeaponEffect::MultiShot))
    {
        FWeaponEffectData MultiData = GetEffectData(EWeaponEffect::MultiShot);
        int32 ExtraShots = FMath::RoundToInt(MultiData.Value1);
        float SpreadAngle = MultiData.Value2;

        // 메인 샷
        if (BulletClass)
            FireBullet(StartPos, Direction);
        else
            FireLineTrace(StartPos, Direction);

        // 추가 샷
        for (int32 i = 0; i < ExtraShots; i++)
        {
            float AngleOffset = FMath::RandRange(-SpreadAngle, SpreadAngle);
            FRotator SpreadRot = Direction.Rotation();
            SpreadRot.Yaw += AngleOffset;
            FVector SpreadDir = SpreadRot.Vector();

            if (BulletClass)
                FireBullet(StartPos, SpreadDir);
            else
                FireLineTrace(StartPos, SpreadDir);
        }
    }
    else
    {
        if (BulletClass)
        {
            FireBullet(StartPos, Direction);
        }
        else
        {

        }
    }
    */

    FireLineTrace(StartPos, Direction);
    PlayFireEffects(GetMuzzleLocation(), Direction);
}

void AAZWeapon::FireLineTrace(FVector StartPos, FVector Direction)
{
    AAZPlayerCharacter* MyPawn = Cast<AAZPlayerCharacter>(GetOwner());
    if (MyPawn == nullptr)
        return;

    FVector EndPos = StartPos + (Direction * MaxDistance);

    // 관통 처리
    bool bPiercing = HasEffect(EWeaponEffect::Piercing);
    int32 PierceCount = 0;
    float PierceDamageReduction = 1.0f;

    if (bPiercing)
    {
        FWeaponEffectData PierceData = GetEffectData(EWeaponEffect::Piercing);
        PierceCount = FMath::RoundToInt(PierceData.Value1);
        PierceDamageReduction = PierceData.Value2;
    }

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(MyPawn);

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    AController* InstigatorController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

    // 관통 시 여러 대상 처리
    if (bPiercing && PierceCount > 0)
    {
        TArray<FHitResult> HitResults;
        GetWorld()->LineTraceMultiByChannel(
            HitResults,
            StartPos,
            EndPos,
            ECC_GameTraceChannel1,
            Params
        );

        float CurrentDamageMultiplier = 1.0f;
        int32 HitCount = 0;

        for (const FHitResult& HitResult : HitResults)
        {
            if (HitCount > PierceCount)
                break;

            if (HitResult.GetActor())
            {
                ProcessSingleHit(HitResult, CurrentDamageMultiplier, InstigatorController);
                CurrentDamageMultiplier *= PierceDamageReduction;
                HitCount++;
            }
        }

        FVector ImpactPoint = HitResults.Num() > 0 ? HitResults.Last().ImpactPoint : EndPos;
        PlayTrailEffect(GetMuzzleLocation(), ImpactPoint);
    }
    else
    {
        // 일반 단일 히트
        FHitResult HitResult;
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            StartPos,
            EndPos,
            ECC_GameTraceChannel1,
            Params
        );

        FVector ImpactPoint = bHit ? HitResult.ImpactPoint : EndPos;
        PlayTrailEffect(GetMuzzleLocation(), ImpactPoint);

        if (bHit && HitResult.GetActor())
        {
            ProcessSingleHit(HitResult, 1.0f, InstigatorController);
        }
    }
}

void AAZWeapon::ProcessSingleHit(const FHitResult& HitResult, float DamageMultiplier, AController* InstigatorController)
{
    AActor* HitActor = HitResult.GetActor();
    if (!HitActor) return;

    PlayImpactEffects(HitResult);

    float FinalDamage = Damage * DamageMultiplier;

    // 크리티컬 체크
    bool bCritical = CheckCritical(FinalDamage);
    if (bCritical)
    {
        PlayCriticalEffects(HitResult.ImpactPoint);
    }

    // 피격 효과 처리 (처형, 스택 등 - 데미지 수정)
    ProcessHitEffects(HitActor, FinalDamage, HitResult);

    // 데미지 적용
    UGameplayStatics::ApplyDamage(
        HitActor,
        FinalDamage,
        InstigatorController,
        this,
        UDamageType::StaticClass()
    );

    // ===== 상태이상 적용 (인터페이스 사용) =====
    ApplyStatusEffectsToTarget(HitActor);

    // 폭발 효과
    if (HasEffect(EWeaponEffect::Explosion))
    {
        FWeaponEffectData ExpData = GetEffectData(EWeaponEffect::Explosion);
        ApplyExplosion(HitResult.ImpactPoint, ExpData.Value1, ExpData.Value2);
    }

    // 처치 체크
    UAZHealthComponent* EnemyHealth = HitActor->FindComponentByClass<UAZHealthComponent>();
    if (EnemyHealth && EnemyHealth->IsDead())
    {
        ProcessKillEffects(HitActor);
    }
}

bool AAZWeapon::CheckCritical(float& OutDamage)
{
    if (HasEffect(EWeaponEffect::CriticalShot))
    {
        FWeaponEffectData CritData = GetEffectData(EWeaponEffect::CriticalShot);
        float Roll = FMath::RandRange(0.0f, 1.0f);

        if (Roll <= CritData.Value1)
        {
            float CritMultiplier = (CritData.Value2 > 0.0f) ? CritData.Value2 : 2.0f;
            OutDamage *= CritMultiplier;
            UE_LOG(LogTemp, Warning, TEXT("CRITICAL HIT! Damage: %.1f (x%.1f)"), OutDamage, CritMultiplier);
            return true;
        }
    }
    return false;
}

void AAZWeapon::ProcessHitEffects(AActor* HitActor, float& OutDamage, const FHitResult& HitResult)
{
    if (!HitActor) return;

    UAZHealthComponent* TargetHealth = HitActor->FindComponentByClass<UAZHealthComponent>();
    if (!TargetHealth) return;

    CheckCritical(OutDamage);

    ApplyStatusEffectsToTarget(HitActor);

    // ===== 처형 (Execute) =====
    if (HasEffect(EWeaponEffect::Execute))
    {
        FWeaponEffectData ExecData = GetEffectData(EWeaponEffect::Execute);
        float HealthPercent = TargetHealth->GetHealthPercent();

        if (HealthPercent <= ExecData.Value1)
        {
            OutDamage = TargetHealth->Hp + 100.0f;
            UE_LOG(LogTemp, Warning, TEXT("EXECUTE! Target HP: %.1f%%"), HealthPercent * 100.f);
            PlayExecuteEffects(HitResult.ImpactPoint);
            return;
        }
    }

    // ===== 스택 데미지 =====
    if (HasEffect(EWeaponEffect::StackDamage))
    {
        FWeaponEffectData StackData = GetEffectData(EWeaponEffect::StackDamage);
        int32 CurrentStacks = AddStackToEnemy(HitActor);

        if (CurrentStacks >= (int32)StackData.Value1)
        {
            OutDamage *= StackData.Value2;
            UE_LOG(LogTemp, Warning, TEXT("STACK BONUS! Stacks: %d, Damage: %.1f"), CurrentStacks, OutDamage);
            EnemyStacks.Remove(HitActor);
        }
    }
}

// ===== 핵심: 인터페이스를 통한 상태이상 적용 =====
void AAZWeapon::ApplyStatusEffectsToTarget(AActor* Target)
{
    if (!Target) return;

    // 출혈 효과 적용
    if (HasEffect(EWeaponEffect::Bleeding))
    {
        FWeaponEffectData BleedData = GetEffectData(EWeaponEffect::Bleeding);

        // 인터페이스 체크 - 누구든 인터페이스만 구현하면 상태이상 받을 수 있음
        if (IAZStatusInterface* StatusInterface = Cast<IAZStatusInterface>(Target))
        {
            FAZStatusEffectPacket BleedPacket = BleedData.ToBleedingPacket();
            StatusInterface->ApplyStatusEffect(BleedPacket, GetOwner());

            UE_LOG(LogTemp, Log, TEXT("[Weapon] Bleeding applied via Interface: %.1f dmg x %.1f sec"),
                BleedData.Value1, BleedData.Value2);
        }
        // 인터페이스 없으면 HealthComponent 직접 시도
        else if (UAZHealthComponent* HealthComp = Target->FindComponentByClass<UAZHealthComponent>())
        {
            FAZStatusEffectPacket BleedPacket = BleedData.ToBleedingPacket();
            HealthComp->ApplyStatusEffect(BleedPacket, GetOwner());

            UE_LOG(LogTemp, Log, TEXT("[Weapon] Bleeding applied via HealthComp: %.1f dmg x %.1f sec"),
                BleedData.Value1, BleedData.Value2);
        }
    }

    // 다른 상태이상 효과도 여기에 추가 가능
    // 예: Poison, Fire 등
}

void AAZWeapon::ApplyExplosion(const FVector& Location, float Radius, float DamageRatio)
{
    PlayExplosionEffects(Location);

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    AController* InstigatorController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(GetOwner());

    UGameplayStatics::ApplyRadialDamage(
        this,
        Damage * DamageRatio,
        Location,
        Radius,
        UDamageType::StaticClass(),
        IgnoreActors,
        this,
        InstigatorController,
        false,
        ECC_GameTraceChannel1
    );

    UE_LOG(LogTemp, Log, TEXT("Explosion: Radius %.1f, Damage %.1f"), Radius, Damage * DamageRatio);
}

void AAZWeapon::ProcessKillEffects(AActor* KilledActor)
{
    if (!KilledActor) return;

    AAZPlayerCharacter* OwnerPlayer = Cast<AAZPlayerCharacter>(GetOwner());
    if (!OwnerPlayer) return;

    // ===== 흡혈 =====
    if (HasEffect(EWeaponEffect::LifeSteal))
    {
        FWeaponEffectData StealData = GetEffectData(EWeaponEffect::LifeSteal);

        UAZHealthComponent* PlayerHealth = OwnerPlayer->HealthComp;
        if (PlayerHealth)
        {
            float HealAmount = PlayerHealth->MaxHp * StealData.Value1;
            PlayerHealth->AddHealth(HealAmount);

            UE_LOG(LogTemp, Warning, TEXT("LIFE STEAL! Healed: %.1f"), HealAmount);
            PlayLifeStealEffects();
        }
    }

    EnemyStacks.Remove(KilledActor);
}

// ==================== 스택 관리 ====================

int32 AAZWeapon::AddStackToEnemy(AActor* Enemy)
{
    if (!Enemy) return 0;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    if (FEnemyStackInfo* StackInfo = EnemyStacks.Find(Enemy))
    {
        StackInfo->CurrentStacks++;
        StackInfo->LastHitTime = CurrentTime;
        return StackInfo->CurrentStacks;
    }
    else
    {
        FEnemyStackInfo NewInfo;
        NewInfo.CurrentStacks = 1;
        NewInfo.LastHitTime = CurrentTime;
        EnemyStacks.Add(Enemy, NewInfo);
        return 1;
    }
}

void AAZWeapon::ClearExpiredStacks()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    TArray<AActor*> ExpiredEnemies;
    for (auto& Pair : EnemyStacks)
    {
        if (CurrentTime - Pair.Value.LastHitTime > StackExpireTime)
        {
            ExpiredEnemies.Add(Pair.Key);
        }
    }

    for (AActor* Enemy : ExpiredEnemies)
    {
        EnemyStacks.Remove(Enemy);
    }
}

// ==================== Projectile ====================

void AAZWeapon::FireBullet(FVector StartPos, FVector Direction)
{
    if (!BulletClass)
        return;

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
        return;

    FVector MuzzleLoc = GetMuzzleLocation();
    FRotator MuzzleRot = Direction.Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = OwnerPawn;

    AAZBullet* Bullet = GetWorld()->SpawnActor<AAZBullet>(
        BulletClass,
        MuzzleLoc,
        MuzzleRot,
        SpawnParams
    );

    if (Bullet)
    {
        float FinalDamage = Damage;
        CheckCritical(FinalDamage);
        Bullet->SetDamage(FinalDamage);
        PRINT_LOG(TEXT("Bullet Spawned! Damage: %.1f"), FinalDamage);
    }
}

// ==================== 기본 함수 ====================

void AAZWeapon::Reload()
{
    CurrentAmmo = MaxAmmo;

    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
    }

    PRINT_LOG(TEXT("Reloaded! Ammo: %d"), CurrentAmmo);
}

void AAZWeapon::Equip(ACharacter* Character)
{
    if (!Character)
        return;

    SetOwner(Character);

    const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("hand_r"));
    if (HandSocket)
    {
        HandSocket->AttachActor(this, Character->GetMesh());
    }
}

void AAZWeapon::PlayEmptySound()
{
    if (EmptyFireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, EmptyFireSound, GetActorLocation());
    }
}

FVector AAZWeapon::GetMuzzleLocation()
{
    if (WeaponMesh)
    {
        const USkeletalMeshSocket* MuzzleSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
        if (MuzzleSocket)
        {
            return MuzzleSocket->GetSocketLocation(WeaponMesh);
        }
    }
    return GetActorLocation();
}

// ==================== 이펙트 ====================

void AAZWeapon::PlayFireEffects(FVector MuzzleLocation, FVector Direction)
{
    if (MuzzleFlashEffect)
    {
        FRotator FireRotation = Direction.Rotation() + MuzzleFlashRotationOffset;  
        FRotator ShootRotation = Direction.Rotation();
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            MuzzleFlashEffect,
            MuzzleLocation,   // 위치는 총구
            ShootRotation     // 회전은 카메라가 보고 있는 방향!
        );
        UE_LOG(LogTemp, Warning, TEXT("Fire Direction: %s"), *Direction.ToString());
    }

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation);
    }
}

void AAZWeapon::PlayImpactEffects(const FHitResult& HitResult)
{
    FVector ImpactPoint = HitResult.ImpactPoint;
    FVector ImpactNormal = HitResult.ImpactNormal;

    if (ImpactEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this,
            ImpactEffect,
            ImpactPoint,
            ImpactNormal.Rotation()
        );
    }

    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ImpactPoint);
    }

    if (BulletDecalMaterial)
    {
        UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
            GetWorld(),
            BulletDecalMaterial,
            DecalSize,
            ImpactPoint,
            ImpactNormal.Rotation(),
            DecalLifetime
        );

        if (Decal)
        {
            Decal->SetFadeScreenSize(0);
        }
    }
}

void AAZWeapon::PlayTrailEffect(const FVector& Start, const FVector& End)
{
    if (BulletTrailEffect)
    {
        FRotator TrailRotation = (End - Start).Rotation() + BulletTrailRotationOffset; 

        UNiagaraComponent* TrailComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this,
            BulletTrailEffect,
            Start,
            TrailRotation,
            BulletTrailScale,  
            true,
            true,
            ENCPoolMethod::None,
            true
        );

        if (TrailComp)
        {
            UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
                TrailComp,
                FName("ImpactPositions"),
                TArray<FVector>({ End })
            );
        }
    }
}

void AAZWeapon::PlayCriticalEffects(const FVector& Location)
{
    if (CriticalHitEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, CriticalHitEffect, Location);
    }

    if (CriticalHitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CriticalHitSound, Location);
    }
}

void AAZWeapon::PlayExecuteEffects(const FVector& Location)
{
    if (ExecuteEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExecuteEffect, Location);
    }
}

void AAZWeapon::PlayLifeStealEffects()
{
    AAZPlayerCharacter* OwnerPlayer = Cast<AAZPlayerCharacter>(GetOwner());
    if (OwnerPlayer && LifeStealEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            LifeStealEffect,
            OwnerPlayer->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }
}

void AAZWeapon::PlayExplosionEffects(const FVector& Location)
{
    if (ExplosionEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplosionEffect, Location);
    }
}