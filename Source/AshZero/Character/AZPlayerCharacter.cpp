
#include "Character/AZPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AZPlayerMoveComponent.h"
#include "Components/AZHealthComponent.h"
#include "Components/AZPlayerFireComponent.h"
#include "Components/AZPlayerInventoryComponent.h"
#include "Components/AZInteractionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Weapon/AZWeapon.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "AZDataAsset.h"
#include "AshZero.h"
#include "UI/HUD/AZStatusBarWidget.h"
#include "System/Player/AZPlayerController.h"
#include "Animation/AZPlayerAnimInstance.h"
#include "NiagaraFunctionLibrary.h" 
#include "Blueprint/UserWidget.h"
#include "kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AZItemUsageComponent.h"
#include "System/GameMode/AZStageGameMode.h"
#include "GameState/AZInGameGameState.h"
#include "Components/CanvasPanel.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"
#include "System/Subsystems/AZSceneSubsystem.h"
#include "System/AZSessionSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AZEquipmentComponent.h"

AAZPlayerCharacter::AAZPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("PlayerCapsule"));

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(RootComponent);
    SpringArmComp->SocketOffset = FVector(0, 70, 90);
    SpringArmComp->TargetArmLength = 300;
    SpringArmComp->bUsePawnControlRotation = true;


    CamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("TpsCamComp"));
    CamComp->SetupAttachment(SpringArmComp);
    CamComp->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = true;

    MoveComp = CreateDefaultSubobject<UAZPlayerMoveComponent>(TEXT("MoveComp"));
    HealthComp = CreateDefaultSubobject<UAZHealthComponent>(TEXT("HealthComp"));
    FireComp = CreateDefaultSubobject<UAZPlayerFireComponent>(TEXT("FireComp"));
    InteractionComp = CreateDefaultSubobject<UAZInteractionComponent>(TEXT("InteractionComp"));

    VignettePostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("VignettePostProcess"));
    VignettePostProcess->SetupAttachment(RootComponent);
    VignettePostProcess->bUnbound = true;
}

void AAZPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 사망 델리게이트 바인딩
    if (HealthComp)
    {
        HealthComp->OnDeath.AddDynamic(this, &AAZPlayerCharacter::OnDeath);
        HealthComp->OnHitRegistered.AddDynamic(this, &AAZPlayerCharacter::HandleOnHit);
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAZSessionSubsystem* SessionSub = GI->GetSubsystem<UAZSessionSubsystem>())
        {
            SessionSub->OnGameStateChanged.AddDynamic(this, &AAZPlayerCharacter::OnGameStateChanged);

            // 초기 상태 적용
            OnGameStateChanged(SessionSub->CurrentGameState);
        }
    }
    SetLocalPlayerMaterialParam();
}

void AAZPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    SetLocalPlayerMaterialParam();
}

void AAZPlayerCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    SetLocalPlayerMaterialParam();
}

void AAZPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAZPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    auto PlayerInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
    if (PlayerInput)
    {
        OnInputBindingDelegate.Broadcast(PlayerInput);
        AAZPlayerController* AZPlayerController = Cast<AAZPlayerController>(GetController());
        if (AZPlayerController)
        {
            if (AZPlayerController->InventoryComp)
            {
                AZPlayerController->InventoryComp->SetupInputBinding(PlayerInput);
            }

            if (AZPlayerController->ItemUsageComp)
            {
                AZPlayerController->ItemUsageComp->SetupInputBinding(PlayerInput);
            }
        }
    }
}
void AAZPlayerCharacter::OnGameStateChanged(EGameState NewState)
{
    if (!MoveComp) return;

    bool bIsLobby = (NewState == EGameState::Lobby);
    MoveComp->SetFreeLookMode(bIsLobby);
}

void AAZPlayerCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();

    if (IsLocallyControlled() && VignetteMaterial && !VignetteMID)
    {
        VignetteMID = UMaterialInstanceDynamic::Create(VignetteMaterial, this);

        if (VignettePostProcess && VignetteMID)
        {
            VignettePostProcess->bUnbound = true;
            VignettePostProcess->AddOrUpdateBlendable(VignetteMID, 1.0f);
            VignetteMID->SetScalarParameterValue(TEXT("KernelSize"), 0.0f);
        }
    }

    if (FireComp)
    {
        FireComp->OwnerCharacter = this;
        FireComp->CamComp = CamComp;
        FireComp->SpringArmComp = SpringArmComp;
        FireComp->InitializeUI();
    }

    APlayerController* MyPlayerController = Cast<APlayerController>(Controller);
    if (!MyPlayerController)
    {
        PRINT_LOG(TEXT("No PlayerController!"));
        return;
    }
    if (!InputDataAsset)
    {
        PRINT_LOG(TEXT("InputDataAsset is NULL!"));
        return;
    }

    // StatusBar UI 생성
    if (IsLocallyControlled() && StatusBarWidgetClass && !StatusBarWidget)
    {
        StatusBarWidget = CreateWidget<UAZStatusBarWidget>(GetWorld(), StatusBarWidgetClass);
        if (StatusBarWidget)
        {
            StatusBarWidget->AddToViewport(0);
            StatusBarWidget->InitializeTeamMemberUI(this);

            if (FireComp)
            {
                FireComp->OnWeaponChanged.RemoveDynamic(StatusBarWidget, &UAZStatusBarWidget::UpdateWeapon);
                FireComp->OnAmmoChanged.RemoveDynamic(StatusBarWidget, &UAZStatusBarWidget::UpdateAmmo);

                FireComp->OnWeaponChanged.AddDynamic(StatusBarWidget, &UAZStatusBarWidget::UpdateWeapon);
                FireComp->OnAmmoChanged.AddDynamic(StatusBarWidget, &UAZStatusBarWidget::UpdateAmmo);

                // 초기 무기 UI 업데이트
                AAZWeapon* SubWeapon = FireComp->GetSubWeapon();
                StatusBarWidget->UpdateWeapon(FireComp->CurrentWeapon, SubWeapon, FireComp->CurrentSlotIndex);
                if (FireComp->CurrentWeapon)
                {
                    StatusBarWidget->UpdateAmmo(FireComp->CurrentWeapon->CurrentAmmo, FireComp->CurrentWeapon->MaxAmmo);
                }
            }
        }
    }

    if (FireComp)
    {
        FireComp->InitializeUI();
    }

    if (MyPlayerController && InputDataAsset)
    {
        auto SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(MyPlayerController->GetLocalPlayer());
        if (SubSystem && InputDataAsset->IMC_TPS)
        {
            SubSystem->AddMappingContext(InputDataAsset->IMC_TPS, 0);
        }
    }

    if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Equals(TEXT("LobbyLevel_Party")))
    {
        AAZPlayerController* PC = Cast<AAZPlayerController>(MyPlayerController);
        if (PC) PC->SetPartyUI();
    }
    else
    {
        FInputModeGameOnly InputMode;
        MyPlayerController->SetInputMode(InputMode);
        MyPlayerController->SetShowMouseCursor(false);

        if (IsLocallyControlled())
        {
            UAZSceneSubsystem* SceneSystem = GetGameInstance()->GetSubsystem<UAZSceneSubsystem>();
            if (SceneSystem) SceneSystem->PlayPlayerSequence();
        }
    }
}


void AAZPlayerCharacter::OnDeath(FVector DeathLocation, AActor* Killer)
{
    bIsDead = true;

    // 모든 상태이상 제거
    for (auto& Pair : StatusTimerMap)
    {
        GetWorldTimerManager().ClearTimer(Pair.Value);
    }
    StatusTimerMap.Empty();

    for (auto& Pair : ActiveDoTEffects)
    {
        GetWorldTimerManager().ClearTimer(Pair.Value.TickTimerHandle);
    }
    ActiveDoTEffects.Empty();

    if (AAZInGameGameState* GameState = Cast<AAZInGameGameState>(GetWorld()->GetGameState()))
    {
        GameState->DecreaseAlivePlayers();
    }

    StartDeathSequence();

    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (GI == nullptr) return;
    UAZSessionSubsystem*  SessionSub= GI->GetSubsystem<UAZSessionSubsystem>();
    if (SessionSub == nullptr) return;
    SessionSub->CurrentGameState = EGameState::Lobby;
    
}

void AAZPlayerCharacter::StartDeathSequence()
{
    // 입력 비활성화
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }

    // 이동 정지
    GetCharacterMovement()->DisableMovement();

    // 충돌 비활성화
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 사망 애니메이션 재생
    if (UAZPlayerAnimInstance* AnimInstance = Cast<UAZPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
    {
        AnimInstance->PlayDeathAnim();
    }
    // 로컬 플레이어 사망 UI 표시
    if (IsLocallyControlled() && DeathWidgetClass)
    {
        DeathWidget = CreateWidget<UUserWidget>(GetWorld(), DeathWidgetClass);
        if (DeathWidget)
        {
            DeathWidget->AddToViewport(100);
        }
    }

    // DeadLoop 시작 시점에 페이드 시작 (타이머)
    GetWorldTimerManager().SetTimer(
        DeathFadeStartHandle,
        this,
        &AAZPlayerCharacter::StartDeathFade,
        DeathAnimDuration,
        false
    );
}

void AAZPlayerCharacter::StartDeathFade()
{
    // VFX 스폰
    if (DeathVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this,
            DeathVFX,
            GetActorLocation(),
            GetActorRotation()
        );
    }

    // 다이나믹 머티리얼 생성
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp && DeathMaterials.Num() > 0)
    {
        int32 NumMats = MeshComp->GetNumMaterials();
        for (int32 i = 0; i < NumMats; i++)
        {
            int32 MatIndex = FMath::Min(i, DeathMaterials.Num() - 1);
            if (DeathMaterials[MatIndex])
            {
                UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(DeathMaterials[MatIndex], this);
                MeshComp->SetMaterial(i, DynMat);
                DynamicMaterials.Add(DynMat);
            }
        }
    }

    // 페이드 타이머 시작
    DeathFadeElapsed = 0.f;
    GetWorldTimerManager().SetTimer(
        DeathFadeTimerHandle,
        this,
        &AAZPlayerCharacter::UpdateDeathFade,
        0.016f,
        true
    );

    // 로비 복귀 타이머
    //GetWorldTimerManager().SetTimer(
    //    LobbyTimerHandle,
    //    this,
    //    &AAZPlayerCharacter::ReturnToLobby,
    //    ReturnToLobbyDelay,
    //    false
    //);
}

void AAZPlayerCharacter::UpdateDeathFade()
{
    DeathFadeElapsed += 0.016f;
    float Alpha = FMath::Clamp(DeathFadeElapsed / FadeOutDuration, 0.f, 1.f);

    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            DynMat->SetScalarParameterValue(FName("Opacity"), 1.f - Alpha);
        }
    }

    if (Alpha >= 1.f)
    {
        FinishDeathSequence();
    }
}

void AAZPlayerCharacter::FinishDeathSequence()
{
    GetWorldTimerManager().ClearTimer(DeathFadeTimerHandle);
    GetMesh()->SetVisibility(false);

    if (IsLocallyControlled())
    {
        DeathWidget->RemoveFromParent();
        //StatusBarWidget->SetVisibility(ESlateVisibility::Collapsed);
        StatusBarWidget->TeamPanel->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (HasAuthority())
    {
        NotifyDeathFinished();
    }
}

//피격
void AAZPlayerCharacter::HandleOnHit(float Damage, AActor* Causer, FVector HitLocation)
{
    // 서버에서 데미지를 확인했으므로 멀티캐스트 호출
    MulticastPlayHitEffect();

    if (IsLocallyControlled())
    {
        PlayLocalHitEffects();  
    }

}

void AAZPlayerCharacter::Server_PlayHitReaction_Implementation(EHitReactionType ReactionType, FVector HitDirection, float KnockbackForce)
{
    Multicast_PlayHitReaction(ReactionType, HitDirection, KnockbackForce);
}

void AAZPlayerCharacter::Multicast_PlayHitReaction_Implementation(EHitReactionType ReactionType, FVector HitDirection, float KnockbackForce)
{
    ExecuteHitReaction(ReactionType, HitDirection, KnockbackForce);
}

void AAZPlayerCharacter::ExecuteHitReaction(EHitReactionType ReactionType, FVector HitDirection, float KnockbackForce)
{
    if (bIsDead || ReactionType == EHitReactionType::None) return;
    if (bIsHitStunned) return;
    bool bShouldCancelActions = (ReactionType != EHitReactionType::LightHit);

    if(bShouldCancelActions)
    {
        bIsHitStunned = true;
        if (FireComp)
        {
            FireComp->CancelReload();
            FireComp->ForceEndAim();
        }
    }


    UAnimMontage* MontageToPlay = nullptr;
    float StunDuration = 0.0f;

    switch (ReactionType)
    {
    case EHitReactionType::LightHit:
        MontageToPlay = LightHitMontage;
        break;

    case EHitReactionType::HeavyHit:
        MontageToPlay = HeavyHitMontage;
        StunDuration = 0.8f;
        break;

    case EHitReactionType::Knockback:
        MontageToPlay = KnockbackMontage;
        StunDuration = 1.2f;

        // 넉백 적용
        LaunchCharacter(HitDirection * KnockbackForce, true, true);
        break;
    }

    UE_LOG(LogTemp, Error, TEXT("[HitReaction] StunDuration: %.2f"), StunDuration);

    // 4. 애니메이션 재생
    if (MontageToPlay)
    {
        PlayAnimMontage(MontageToPlay);
    }

    // 5. 스턴 해제 타이머
    GetWorldTimerManager().ClearTimer(HitStunTimerHandle);
    GetWorldTimerManager().SetTimer(
        HitStunTimerHandle,
        [this]()
        {
            bIsHitStunned = false;
            UE_LOG(LogTemp, Error, TEXT("[HitReaction] TIMER END - bIsHitStunned = FALSE"));
        },
        StunDuration,
        false
    );
}

void AAZPlayerCharacter::PlayItemUseAnim_Server_Implementation()
{
    PlayItemUseAnim_Multicast();
}

void AAZPlayerCharacter::PlayItemUseAnim_Multicast_Implementation()
{
    if (UAZPlayerAnimInstance* AnimInst = Cast<UAZPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
    {
        if (AnimInst->PlayUseItemAnim())
        {
            AAZPlayerController* PC = Cast<AAZPlayerController>(GetController());
            if (PC && PC->EquipmentComp)
            {
                PC->EquipmentComp->SubUsedQuickSlotItem();
                PC->EquipmentComp->SetUsedQuickSlotIndex(ESlotIndex::Slot_0);
            }
        }
    }
}

void AAZPlayerCharacter::PlayItemUseVFX_Server_Implementation(UNiagaraSystem* VFX)
{
    PlayItemUseVFX_Multicast(VFX);
}
void AAZPlayerCharacter::PlayItemUseVFX_Multicast_Implementation(UNiagaraSystem* VFX)
{
    if (VFX)
    {
        FVector SpawnVector = FVector(0.0f, 0.0f, 100.0f); // 위치 조정 필요 시 수정
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            VFX,
            GetMesh(),
            NAME_None,
            SpawnVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
}

void AAZPlayerCharacter::PlaySFX_Server_Implementation(ESFXType SFXType)
{
    PlaySFX_Multicast(SFXType);
}

void AAZPlayerCharacter::PlaySFX_Multicast_Implementation(ESFXType SFXType)
{
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (GI == nullptr) return;
    UAZSoundManagerSubsystem* SoundManager = GI->GetSubsystem<UAZSoundManagerSubsystem>();
    if (SoundManager == nullptr) return;
    
    SoundManager->PlaySFX(this, SFXType);
}

void AAZPlayerCharacter::PlayLocalHitEffects()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC && HitCameraShakeClass)
    {
        PC->ClientStartCameraShake(HitCameraShakeClass);
    }
    if (PC && DamageOverlayClass)
    {
        UUserWidget* TempOverlay = CreateWidget<UUserWidget>(GetWorld(), DamageOverlayClass);
        if (TempOverlay)
        {
            TempOverlay->AddToViewport();

            // 블루프린트의 PlayHitAnim 함수 호출
            UFunction* AnimFunc = TempOverlay->FindFunction(FName("PlayHitAnim"));
            if (AnimFunc)
            {
                TempOverlay->ProcessEvent(AnimFunc, nullptr);
            }
        }
        /*
        if (DamageOverlayWidget)
        {
            UFunction* AnimFunc = DamageOverlayWidget->FindFunction(FName("PlayHitAnim"));
            if (AnimFunc)
            {
                DamageOverlayWidget->ProcessEvent(AnimFunc, nullptr);
            }
        }*/
    }
}

void AAZPlayerCharacter::MulticastPlayHitEffect_Implementation()
{
    if (BloodVFX && GetMesh())
    {
        // 지정한 소켓 위치에 나이아가라 스폰
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            BloodVFX,
            GetMesh(),
            BloodSocketName,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }
}

//void AAZPlayerCharacter::ReturnToLobby()
//{
//    if (APlayerController* PC = Cast<APlayerController>(GetController()))
//    {
//        UGameplayStatics::OpenLevel(this, FName("LobbyLevel_Party"));
//    }
//}HandleOnHit

bool AAZPlayerCharacter::CanPerformAction() const
{
    return !bIsDead;
}

bool AAZPlayerCharacter::IsDoTEffect(EAZStatusType StatusType) const
{
    switch (StatusType)
    {
    case EAZStatusType::Poison:
    case EAZStatusType::Bleeding:
    case EAZStatusType::Fire:
    case EAZStatusType::Starvation:
    case EAZStatusType::Dehydration:
        return true;
    default:
        return false;
    }
}

// ===== IAZStatusInterface 구현 =====
void AAZPlayerCharacter::ApplyStatusEffect(const FAZStatusEffectPacket& Packet, AActor* Causer)
{
    // 서버에서만 상태 변화를 주도함
    if (!HasAuthority()) return;
    if (bIsDead) return;

    EAZStatusType Type = Packet.StatusType;

    // DoT 효과인 경우 별도 처리
    if (IsDoTEffect(Type))
    {
        // 이미 같은 DoT가 있으면 지속시간 갱신
        if (ActiveDoTEffects.Contains(Type))
        {
            if (Packet.Duration > 0.0f)
            {
                ActiveDoTEffects[Type].RemainingDuration = Packet.Duration;
            }
            ActiveDoTEffects[Type].EffectPacket = Packet;
        }
        else
        {
            StartDoTEffect(Packet);
        }
        return;
    }

    // 배율이 1.0이면 효과 제거 (정상 상태로 복귀)
    if (FMath::IsNearlyEqual(Packet.StatMultiplier, 1.0f))
    {
        RemoveStatusEffect(Type);
        return;
    }

    // 이미 같은 상태이상이 걸려있는 경우
    if (StatusTimerMap.Contains(Type))
    {
        // 지속 시간이 있는 경우라면 타이머 갱신 (시간 연장)
        if (Packet.Duration > 0.0f)
        {
            GetWorldTimerManager().ClearTimer(StatusTimerMap[Type]);

            FTimerHandle NewHandle;
            GetWorldTimerManager().SetTimer(
                NewHandle,
                FTimerDelegate::CreateUObject(this, &AAZPlayerCharacter::RemoveStatusEffect, Type),
                Packet.Duration,
                false
            );
            StatusTimerMap[Type] = NewHandle;
        }
        return;
    }

    // 효과 즉시 적용
    ExecuteStatusEffect(Packet);

    // 지속 시간이 있다면 타이머 설정
    if (Packet.Duration > 0.0f)
    {
        FTimerHandle NewHandle;
        GetWorldTimerManager().SetTimer(
            NewHandle,
            FTimerDelegate::CreateUObject(this, &AAZPlayerCharacter::RemoveStatusEffect, Type),
            Packet.Duration,
            false
        );
        StatusTimerMap.Add(Type, NewHandle);
    }
    else
    {
        // Duration 0 = 무한 지속, 빈 타이머로 등록 (중복 방지용)
        StatusTimerMap.Add(Type, FTimerHandle());
    }
}

void AAZPlayerCharacter::ExecuteStatusEffect(FAZStatusEffectPacket EffectPacket)
{
    if (bIsDead) return;

    switch (EffectPacket.StatusType)
    {
    case EAZStatusType::SpeedBoost:
        if (MoveComp) { MoveComp->SetSpeedMultiplier(EffectPacket.StatMultiplier); }
        break;

    case EAZStatusType::Starvation:
        if (HealthComp) { HealthComp->SetMaxHpMultiplier(EffectPacket.StatMultiplier); }
        break;

    case EAZStatusType::Dehydration:
        if (MoveComp) { MoveComp->SetSpeedMultiplier(EffectPacket.StatMultiplier); }
        break;

    case EAZStatusType::MaxHpUp:
        if (HealthComp) { HealthComp->SetMaxHpMultiplier(EffectPacket.StatMultiplier); }
        break;

    case EAZStatusType::Poison:
        break;

    case EAZStatusType::PowerUp:
        // TODO: 공격력 버프 적용
        break;

    case EAZStatusType::MaxStaminaUp:
        // TODO: 최대 스태미나 버프 적용
        break;

    case EAZStatusType::Invincible:
        // TODO: 무적 상태 적용
        break;

    default:
        break;
    }
}

void AAZPlayerCharacter::RemoveStatusEffect(EAZStatusType StatusType)
{
    if (!HasAuthority()) return;

    // 타이머 정리
    if (StatusTimerMap.Contains(StatusType))
    {
        GetWorldTimerManager().ClearTimer(StatusTimerMap[StatusType]);
        StatusTimerMap.Remove(StatusType);
    }

    // 효과 복구
    switch (StatusType)
    {
    case EAZStatusType::SpeedBoost:
        if (MoveComp) { MoveComp->SetSpeedMultiplier(1.0f); };
        break;

    case EAZStatusType::Starvation:
    case EAZStatusType::MaxHpUp:
        if (HealthComp) { HealthComp->ResetMaxHpMultiplier(); }
        break;

    case EAZStatusType::PowerUp:
        // if (FireComp) FireComp->SetDamageMultiplier(1.0f);
        break;

    case EAZStatusType::MaxStaminaUp:
        // if (MoveComp) MoveComp->SetMaxStaminaMultiplier(1.0f);
        break;

    case EAZStatusType::Invincible:
        // bIsInvincible = false;
        break;

    default:
        break;
    }
}

// ============================================================
// DoT (Damage over Time) 시스템
// ============================================================

void AAZPlayerCharacter::StartDoTEffect(const FAZStatusEffectPacket& EffectPacket)
{
    EAZStatusType Type = EffectPacket.StatusType;

    FActiveDoTEffect NewDoT;
    NewDoT.EffectPacket = EffectPacket;
    NewDoT.RemainingDuration = EffectPacket.Duration;

    // 틱 타이머 시작
    GetWorldTimerManager().SetTimer(
        NewDoT.TickTimerHandle,
        FTimerDelegate::CreateUObject(this, &AAZPlayerCharacter::ProcessDoTTick, Type),
        EffectPacket.TickInterval,
        true,  // 반복
        0.0f   // 즉시 시작
    );

    ActiveDoTEffects.Add(Type, NewDoT);

    UE_LOG(LogTemp, Log, TEXT("DoT Started: %d, Duration: %.1f, Tick: %.1f, DamagePerTick: %.1f"),
        (int32)Type, EffectPacket.Duration, EffectPacket.TickInterval, EffectPacket.ValuePerTick);
}

void AAZPlayerCharacter::ProcessDoTTick(EAZStatusType StatusType)
{
    if (bIsDead)
    {
        StopDoTEffect(StatusType);
        return;
    }

    if (!ActiveDoTEffects.Contains(StatusType)) return;

    FActiveDoTEffect& DoT = ActiveDoTEffects[StatusType];

    // 데미지 적용
    if (DoT.EffectPacket.ValuePerTick > 0.0f)
    {
        UGameplayStatics::ApplyDamage(
            this,
            DoT.EffectPacket.ValuePerTick,
            nullptr,  // InstigatorController
            nullptr,  // DamageCauser
            nullptr   // DamageType
        );

        UE_LOG(LogTemp, Log, TEXT("DoT Tick: %d dealt %.1f damage"), (int32)StatusType, DoT.EffectPacket.ValuePerTick);
    }

    // Duration > 0인 경우만 시간 체크 (0이면 무한 지속)
    if (DoT.EffectPacket.Duration > 0.0f)
    {
        DoT.RemainingDuration -= DoT.EffectPacket.TickInterval;

        if (DoT.RemainingDuration <= 0.0f)
        {
            StopDoTEffect(StatusType);
        }
    }
}

void AAZPlayerCharacter::StopDoTEffect(EAZStatusType StatusType)
{
    if (!ActiveDoTEffects.Contains(StatusType)) return;

    FActiveDoTEffect& DoT = ActiveDoTEffects[StatusType];
    GetWorldTimerManager().ClearTimer(DoT.TickTimerHandle);
    ActiveDoTEffects.Remove(StatusType);

}

void AAZPlayerCharacter::NotifyDeathFinished()
{
    AAZPlayerController* PC = Cast<AAZPlayerController>(GetController());

    if (PC)
    {
        PC->HandleDeathFinished();
    }
}

void AAZPlayerCharacter::SetLocalPlayerMaterialParam()
{
    bool bIsLocal = IsLocallyControlled();
    float TargetParamValue = IsLocallyControlled() ? 0.0f : 1.0f;

    USkeletalMeshComponent* MyMesh = GetMesh();
    if (MyMesh)
    {
        if (LocalHairMaterial && RemoteHairMaterial)
            MyMesh->SetMaterial(5, bIsLocal ? LocalHairMaterial : RemoteHairMaterial);
        const int32 MaterialCount = MyMesh->GetNumMaterials();
        for (int32 i = 0; i < MaterialCount; i++)
        {
            UMaterialInstanceDynamic* DynMat = MyMesh->CreateDynamicMaterialInstance(i);
            if (DynMat)
            {
                DynMat->SetScalarParameterValue(FName("IsLocalPlayer"), TargetParamValue);
            }
        }
    }
}
