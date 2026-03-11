// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AZEnemyBase.h"
#include "Enemy/AZEnemyFSM.h"
#include "Enemy/AZAIController.h"
#include "Enemy/UI/AZEnemyHealthUIComponent.h"
#include "Components/AZHealthComponent.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NavigationInvokerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AshZero.h"
#include "NavigationSystem.h"
#include "Engine/OverlapResult.h"
#include "BrainComponent.h"

// Sets default values
AAZEnemyBase::AAZEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("EnemyCapsule"));
    GetMesh()->SetCollisionProfileName(TEXT("EnemyMesh"));

    // AI Controller 자동 설정
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAZAIController::StaticClass();

    HealthComponent = CreateDefaultSubobject<UAZHealthComponent>(TEXT("HealthComponent"));
    HealthUIComponent = CreateDefaultSubobject<UAZEnemyHealthUIComponent>(TEXT("HealthUIComponent"));
    FSMComponent = CreateDefaultSubobject<UAZEnemyFSM>(TEXT("FSM"));

    //NavigationInvoker
    NavInvokerComponent = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
    NavInvokerComponent->SetGenerationRadii(500.0f, 800.0f);

    AmbientSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientSound"));
    AmbientSoundComponent->SetupAttachment(RootComponent);
    AmbientSoundComponent->bAutoActivate = false;

}

void AAZEnemyBase::BeginPlay()
{
	Super::BeginPlay();

    if (HealthComponent && HealthUIComponent)
    {
        if (!HealthComponent->OnHealthChanged.IsAlreadyBound(HealthUIComponent, &UAZEnemyHealthUIComponent::OnHealthChanged))
        {
            HealthComponent->OnHealthChanged.AddDynamic(HealthUIComponent, &UAZEnemyHealthUIComponent::OnHealthChanged);
        }
    }

    if (HasAuthority())
    {
        if (HealthComponent)
        {
            HealthComponent->OnDeath.AddDynamic(this, &AAZEnemyBase::OnDeath);
        }
        OnTakeAnyDamage.AddDynamic(this, &AAZEnemyBase::OnTakeAnyDamageCallback);
    }

    if (AmbientSoundComponent && AmbientLoopSound)
    {
        AmbientSoundComponent->SetSound(AmbientLoopSound);
        AmbientSoundComponent->SetVolumeMultiplier(AmbientSoundVolume);
        if (AmbientSoundAttenuation)
        {
            AmbientSoundComponent->AdjustAttenuation(AmbientSoundAttenuation->Attenuation);
        }
        AmbientSoundComponent->Play();
    }

    PreviousYaw = GetActorRotation().Yaw;
}

void AAZEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentYaw = GetActorRotation().Yaw;
    float YawDelta = FMath::FindDeltaAngleDegrees(PreviousYaw, CurrentYaw);
    CurrentTurnRate = YawDelta / DeltaTime; 
    PreviousYaw = CurrentYaw;
}

void AAZEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAZEnemyBase, bIsDead);
}

void AAZEnemyBase::OnRep_IsDead()
{
    if (bIsDead)
    {
        // 클라이언트에서 사망 처리
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCharacterMovement()->DisableMovement();
    }
}

void AAZEnemyBase::OnTakeAnyDamageCallback(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (!HasAuthority() || bIsDead) return;

    if (FSMComponent && FSMComponent->State == EEnemyState::Damage)
    {
        FVector HitLocation = DamagedActor->GetActorLocation();
        OnPlayHitEffect(HitLocation);
        return;
    }

    if (FSMComponent)
    {
        AActor* Attacker = nullptr;
        if (InstigatedBy)
        {
            Attacker = InstigatedBy->GetPawn();
        }

        FSMComponent->OnDamageReceived(Damage, Attacker);
    }

    FVector HitLocation = DamagedActor->GetActorLocation();
    OnPlayHitEffect(HitLocation);
}

void AAZEnemyBase::OnDeath(FVector DeathLocation, AActor* Killer)
{
    if (!HasAuthority()) return;

    bIsDead = true;

	// 사망 시 모든 몽타주 정지
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->StopAllMontages(0.0f);
    }

    // AI Controller 정지
    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        AIController->StopMovement();
        AIController->SetFocus(nullptr);  // 타겟 바라보기 해제
    }

    // FSM 정지
    if (FSMComponent)
    {
        FSMComponent->ServerSetState(EEnemyState::Die); 
        AmbientSoundComponent->FadeOut(0.0f, 0.0f);
    }

    // 충돌 비활성화
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // 사망 애니메이션
    if (DeathMontage)
    {
        MulticastPlayMontage(DeathMontage);
    }

    // 사망 이펙트
    if (DeathVFX)
    {
        MulticastPlayDeathEffect(GetActorLocation());
    }

    if (DeathMaterials.Num() > 0)
    {
        GetWorldTimerManager().SetTimer(
            DeathFadeDelayHandle,
            this,
            &AAZEnemyBase::StartDeathFade,
            DeathFadeDelay,
            false
        );
    }
    else
    {
        // DeathMaterials 없으면 기존 방식 (바로 사라짐)
        SetLifeSpan(DeathLifeSpan);
    }
}

void AAZEnemyBase::MulticastPlayMontage_Implementation(UAnimMontage* MontageToPlay, float PlayRate)
{
    if (MontageToPlay)
    {
        float Duration = PlayAnimMontage(MontageToPlay, PlayRate);
    }
}


void AAZEnemyBase::SetMovementSpeed(float newSpeed)
{
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = newSpeed;
    }
}

float AAZEnemyBase::GetMovementSpeed() const
{
    return GetVelocity().Size();
}

float AAZEnemyBase::GetMovementDirection() const
{
    FVector Velocity = GetVelocity();
    if (Velocity.IsNearlyZero()) return 0.0f;

    // Z축은 무시하고 2D 평면 속도만 사용합니다.
    Velocity.Z = 0.0f;

    // 2. 속도가 거의 0이면 (움직이지 않는 상태) 0을 반환합니다.
    if (Velocity.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }

    // 3. 캐릭터의 전방 방향 벡터를 가져옵니다.
    FVector ForwardVector = GetActorForwardVector();

    // 4. 안전하게 정규화된 속도 벡터를 가져옵니다.
    FVector NormalVelocity = Velocity.GetSafeNormal();

    // 5. FMath::Atan2를 사용하여 각도(라디안)를 계산합니다.
    // Atan2(y, x)에서 y는 Cross Product (회전 방향), x는 Dot Product (회전 크기)입니다.

    // YawCross: 캐릭터 전방을 기준으로 속도 벡터가 어느 방향으로 치우쳤는지 (음수/양수)
    float YawCross = FVector::CrossProduct(ForwardVector, NormalVelocity).Z;

    // YawDot: 캐릭터 전방과 속도 벡터가 얼마나 비슷한지 (각도 크기)
    float YawDot = FVector::DotProduct(ForwardVector, NormalVelocity);

    // 라디안 각도 계산
    float DirectionInRadians = FMath::Atan2(YawCross, YawDot);

    // 6. 라디안을 디그리(도)로 변환하여 반환합니다.
    return FMath::RadiansToDegrees(DirectionInRadians);
}

float AAZEnemyBase::GetTurnRate() const
{
    return CurrentTurnRate;
}

void AAZEnemyBase::StartDeathFade()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp || DeathMaterials.Num() == 0) return;
    
    DropLootBox();

    // 다이나믹 머티리얼 생성
    int32 NumMats = MeshComp->GetNumMaterials();
    for (int32 i = 0; i < NumMats; i++)
    {
        int32 MatIndex = FMath::Min(i, DeathMaterials.Num() - 1);
        if (DeathMaterials[MatIndex])
        {
            UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(DeathMaterials[MatIndex], this);
            MeshComp->SetMaterial(i, DynMat);
            DynamicDeathMaterials.Add(DynMat);
        }
    }

    // 페이드 타이머 시작
    DeathFadeElapsed = 0.f;
    GetWorldTimerManager().SetTimer(
        DeathFadeTimerHandle,
        this,
        &AAZEnemyBase::UpdateDeathFade,
        0.016f,
        true
    );
}

void AAZEnemyBase::UpdateDeathFade()
{
    DeathFadeElapsed += 0.016f;
    float Alpha = FMath::Clamp(DeathFadeElapsed / DeathFadeDuration, 0.f, 1.f);

    for (UMaterialInstanceDynamic* DynMat : DynamicDeathMaterials)
    {
        if (DynMat)
        {
            if (DeathFadeType == EDeathFadeType::Opacity)
            {
                // Opacity: 1 → 0
                DynMat->SetScalarParameterValue(OpacityParameterName, 1.f - Alpha);
            }
            else // Dissolve
            {
                // Dissolve: 0 → 1
                DynMat->SetScalarParameterValue(DissolveParameterName, Alpha);
            }
        }
    }

    if (Alpha >= 1.f)
    {
        FinishDeathFade();
    }
}

void AAZEnemyBase::FinishDeathFade()
{
    GetWorldTimerManager().ClearTimer(DeathFadeTimerHandle);
    GetMesh()->SetVisibility(false);

    Destroy();
}


void AAZEnemyBase::DropLootBox()
{
    if (ItemDropClass)
    {
        //박스 크기 구하기
        FVector BoxExtent = FVector(40.0f, 40.0f, 40.0f);
        AActor* DefaultActor = ItemDropClass->GetDefaultObject<AActor>();
        if (DefaultActor)
        {
            FVector Origin;
            FVector CalculatedExtent;

            DefaultActor->GetActorBounds(true, Origin, CalculatedExtent);
            if (!CalculatedExtent.IsZero())
            {
                BoxExtent = CalculatedExtent;
            }
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        FVector SpawnLocation = GetActorLocation();

        UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (!NavSystem) return;

        constexpr int32 MaxTryCount = 10;
        constexpr float RandomRadius = 150.f;

        for (int32 i = 0; i < MaxTryCount; i++)
        {
            FVector TestLocation = SpawnLocation;

            if (i > 0)
            {
                FVector RandomOffset = FMath::VRand();
                RandomOffset.Z = 0.f;
                RandomOffset.Normalize();

                TestLocation += RandomOffset * RandomRadius;

            }

            FNavLocation NavLocation;
            bool bOnNav = NavSystem->ProjectPointToNavigation(
                TestLocation,
                NavLocation,
                FVector(400.0f, 400.0f, 500.0f)
            );

            if (bOnNav)
            {
                TestLocation = NavLocation.Location;
            }

            TArray<FOverlapResult> Overlaps;
            FCollisionShape CheckShape = FCollisionShape::MakeBox(BoxExtent);

            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);

            bool bOverlap = GetWorld()->OverlapMultiByChannel(
                Overlaps,
                TestLocation,
                FQuat::Identity,
                ECC_WorldDynamic,
                CheckShape,
                QueryParams
            );

            bool bBlocked = false;

            if (bOverlap)
            {
                for (const FOverlapResult& Result : Overlaps)
                {
                    AActor* OverlappedActor = Result.GetActor();
                    if (OverlappedActor && OverlappedActor->IsA(ItemDropClass))
                    {
                        bBlocked = true;
                        break;
                    }
                }
            }

            if (!bBlocked)
            {
                SpawnLocation = TestLocation;
                break;
            }
        }

        GetWorld()->SpawnActor<AActor>(
            ItemDropClass,
            SpawnLocation,
            GetActorRotation(),
            SpawnParams
        );
    }
}


void AAZEnemyBase::MulticastPlayDeathEffect_Implementation(FVector Location)
{
    
    if (DeathVFX)
    {
        UNiagaraComponent * SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
            DeathVFX,
            GetMesh(),                    // 메시 컴포넌트에 붙임
            NAME_None,                    // 소켓 없음
            DeathVFXOffset,               // 오프셋
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true,                         // bAutoDestroy
            true,                         // bAutoActivate
            ENCPoolMethod::None,
            true                          // bPreCullCheck
        );
    }


    // 2. 사운드
    if (DeathSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            DeathSound,
            Location
        );
    }

        // 3. BP 이벤트 (추가 커스텀용)
    OnPlayDeathEffect();
    
}

AActor* AAZEnemyBase::GetCurrentTarget() const
{
    if (FSMComponent && FSMComponent->CurrentTarget)
    {
        return FSMComponent->CurrentTarget;
    }
    return nullptr;
}

void AAZEnemyBase::SetRoomDormant(bool bDormant)
{
    if (!HasAuthority()) return;
    if (bIsDead) return;
    if (bIsRoomDormant == bDormant) return;

    bIsRoomDormant = bDormant;
    ApplyRoomDormantState(bDormant);

    ForceNetUpdate();
}

void AAZEnemyBase::OnRep_IsRoomDormant()
{
    // 클라에서도 동일하게 적용(틱/이동/충돌은 기본적으로 복제되지 않음)
    ApplyRoomDormantState(bIsRoomDormant);
}

void AAZEnemyBase::ApplyRoomDormantState(bool bDormant)
{
    if (bIsDead) return;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();

    if (!MoveComp || !CapsuleComp) return;

    if (bDormant)
    {
        // 캐시
        bCachedActorTickEnabled = IsActorTickEnabled();
        bCachedMovementTickEnabled = MoveComp->IsComponentTickEnabled();
        CachedGravityScale = MoveComp->GravityScale;
        CachedMovementMode = MoveComp->MovementMode;

        CachedCapsuleCollision = CapsuleComp->GetCollisionEnabled();

        if (FSMComponent) bCachedFSMTickEnabled = FSMComponent->IsComponentTickEnabled();
        if (NavInvokerComponent) bCachedNavInvokerTickEnabled = NavInvokerComponent->IsComponentTickEnabled();

        // 이동/낙하 정지
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement();          // MOVE_None
        MoveComp->GravityScale = 0.0f;
        MoveComp->SetComponentTickEnabled(false);

        // AI/FSM 정지
        if (AAIController* AI = Cast<AAIController>(GetController()))
        {
            AI->StopMovement();
            AI->SetFocus(nullptr);

            if (UBrainComponent* Brain = AI->GetBrainComponent())
            {
                Brain->StopLogic(TEXT("RoomDormant"));
            }
        }

        if (FSMComponent) FSMComponent->SetComponentTickEnabled(false);
        if (NavInvokerComponent) NavInvokerComponent->SetComponentTickEnabled(false);

        // 충돌 끔(필수는 아니지만, 룸 숨김 상태에서 불필요한 상호작용 방지)
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // 액터 틱 끔
        SetActorTickEnabled(false);
    }
    else
    {
        // 복구
        CapsuleComp->SetCollisionEnabled(CachedCapsuleCollision);

        MoveComp->GravityScale = CachedGravityScale;
        MoveComp->SetComponentTickEnabled(bCachedMovementTickEnabled);

        // DisableMovement로 MOVE_None가 되었으니 원래 모드 복구
        // (원래도 MOVE_None였던 경우 안전하게 Walking으로)
        const EMovementMode RestoreMode = (CachedMovementMode == MOVE_None) ? MOVE_Walking : CachedMovementMode.GetValue();
        MoveComp->SetMovementMode(RestoreMode);

        if (AAIController* AI = Cast<AAIController>(GetController()))
        {
            if (UBrainComponent* Brain = AI->GetBrainComponent())
            {
                Brain->RestartLogic();
            }
        }

        if (FSMComponent) FSMComponent->SetComponentTickEnabled(bCachedFSMTickEnabled);
        if (NavInvokerComponent) NavInvokerComponent->SetComponentTickEnabled(bCachedNavInvokerTickEnabled);

        SetActorTickEnabled(bCachedActorTickEnabled);
    }
}
