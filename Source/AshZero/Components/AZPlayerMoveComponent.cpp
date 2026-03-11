
#include "Components/AZPlayerMoveComponent.h"
#include "Components/AZPlayerFireComponent.h"
#include "Components/AZHealthComponent.h"   
#include "Character/AZPlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AZPlayerAnimInstance.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AZDataAsset.h"
#include "AZGameplayTags.h"
#include "Weapon/AZWeapon.h"
#include "AshZero.h"
#include "Net/UnrealNetwork.h"

UAZPlayerMoveComponent::UAZPlayerMoveComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UAZPlayerMoveComponent::BeginPlay()
{
    Super::BeginPlay();

    MaxStamina = BaseStamina;
    CurrentStamina = MaxStamina;
    OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

    if (CharacterMoveComp)
    {
        CharacterMoveComp->MaxWalkSpeed = WalkSpeed;
    }
}

void UAZPlayerMoveComponent::SetupInputBinding(UEnhancedInputComponent* PlayerInput)
{
    if (!PlayerInput || !OwnerCharacter || !OwnerCharacter->InputDataAsset)
    {
        return;
    }

    UAZDataAsset* InputData = OwnerCharacter->InputDataAsset;
    if (InputData == nullptr) return;

    PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Look), ETriggerEvent::Triggered, this, &UAZPlayerMoveComponent::Look);
    PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Move), ETriggerEvent::Triggered, this, &UAZPlayerMoveComponent::Move);
    PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Run), ETriggerEvent::Started, this, &UAZPlayerMoveComponent::SprintStart);
    PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Run), ETriggerEvent::Completed, this, &UAZPlayerMoveComponent::SprintEnd);
    PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Roll), ETriggerEvent::Started, this, &UAZPlayerMoveComponent::Roll);

    PRINT_LOG(TEXT("Input Binding Setup Complete"));
}

void UAZPlayerMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateStamina(DeltaTime);

    if (bIsRecoveringFromRoll && OwnerCharacter)
    {
        RecoveryElapsed += DeltaTime;
        float Alpha = FMath::Clamp(RecoveryElapsed / RotateDuration, 0.f, 1.f);
        float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, RollEaseExponent);

        FRotator NewRotator = FMath::Lerp(RollStartRotation, RollTargetRotation, EasedAlpha);
        OwnerCharacter->SetActorRotation(FRotator(0.f, NewRotator.Yaw, 0.f));

        if (Alpha >= 1.0f)
        {
            EndRoll();
        }
    }
}

void UAZPlayerMoveComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UAZPlayerMoveComponent, WalkSpeed);
    DOREPLIFETIME(UAZPlayerMoveComponent, RunSpeed);
    DOREPLIFETIME(UAZPlayerMoveComponent, CurrentStamina);
    DOREPLIFETIME(UAZPlayerMoveComponent, MaxStamina);
}

void UAZPlayerMoveComponent::OnRep_UpdateSpeed()
{
    if (CharacterMoveComp)
    {
        CharacterMoveComp->MaxWalkSpeed = WalkSpeed;
    }
}

void UAZPlayerMoveComponent::OnRep_Stamina()
{
    OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UAZPlayerMoveComponent::OnRep_MaxStamina()
{
    OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UAZPlayerMoveComponent::SetSpeedMultiplier(float Multiplier)
{
    SpeedMultiplier = Multiplier;

    if (OwnerCharacter)
    {
        OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SpeedMultiplier;
    }
}

// ==================== 이동 입력 ====================

void UAZPlayerMoveComponent::Look(const FInputActionValue& InputValue)
{
    const FVector2D Value = InputValue.Get<FVector2D>();

    if (Value.X != 0.0f)
    {
        OwnerCharacter->AddControllerYawInput(Value.X);
    }

    if (Value.Y != 0.0f)
    {
        OwnerCharacter->AddControllerPitchInput(Value.Y);
    }
}

void UAZPlayerMoveComponent::Move(const FInputActionValue& InputValue)
{
    if (bIsRolling) return;

    if (OwnerCharacter && OwnerCharacter->bIsHitStunned) return;

    const FVector2D Value = InputValue.Get<FVector2D>();
    const FRotator MovementRotation(0.0f, OwnerCharacter->Controller->GetControlRotation().Yaw, 0.0f);
    CurrentMoveInput = Value;

    if (Value.X != 0.0f)
    {
        const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
        OwnerCharacter->AddMovementInput(MovementDirection, Value.X);
    }

    if (Value.Y != 0.0f)
    {
        const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
        OwnerCharacter->AddMovementInput(MovementDirection, Value.Y);
    }
}

// ==================== 달리기 ====================

void UAZPlayerMoveComponent::SprintStart(const FInputActionValue& InputValue)
{
    if (bIsRolling) return;
    if (CurrentStamina <= 0.f) return;

    // 조준 중이면 달리기 불가
    if (OwnerCharacter && OwnerCharacter->FireComp && OwnerCharacter->FireComp->bIsAiming)
    {
        return;
    }

    if (OwnerCharacter && OwnerCharacter->bIsHitStunned) return;

    Server_SprintStart();
    bIsRun = true;
}

void UAZPlayerMoveComponent::SprintEnd(const FInputActionValue& InputValue)
{
    Server_SprintEnd();
    bIsRun = false;
}

void UAZPlayerMoveComponent::Interact(const FInputActionValue& InputValue)
{
    Server_Interact();
}

void UAZPlayerMoveComponent::Server_SprintStart_Implementation()
{
    bIsRun = true;
    Multicast_SprintStart();
}

void UAZPlayerMoveComponent::Server_SprintEnd_Implementation()
{
    bIsRun = false;
    Multicast_SprintEnd();
}

void UAZPlayerMoveComponent::Multicast_SprintStart_Implementation()
{
    if (CharacterMoveComp)
    {
        CharacterMoveComp->MaxWalkSpeed = RunSpeed;
    }
}

void UAZPlayerMoveComponent::Multicast_SprintEnd_Implementation()
{
    if (CharacterMoveComp)
    {
        CharacterMoveComp->MaxWalkSpeed = WalkSpeed;
    }
}

void UAZPlayerMoveComponent::Server_Interact_Implementation()
{
    // TODO: 상호작용 구현
}

// ==================== 구르기 ====================

void UAZPlayerMoveComponent::Roll(const FInputActionValue& InputValue)
{
	// 자유 시점 모드에서는 구르기 불가
    if (bIsFreeLookMode) return;
    // 이미 구르는 중이면 무시
    if (bIsRolling) return;

    // 스왑 중이면 구르기 불가
    if (OwnerCharacter && OwnerCharacter->FireComp && OwnerCharacter->FireComp->bIsSwapping)
        return;

    // 쿨타임 체크
    if (!bCanRoll) return;

    // 스테미나 체크
    if (CurrentStamina < RollStaminaCost) return;

    if (OwnerCharacter && OwnerCharacter->bIsHitStunned) return;

    //  구르기 시작 전 모든 행동 중단
    if (OwnerCharacter && OwnerCharacter->FireComp)
    {
        // 연사 중단
        OwnerCharacter->FireComp->InputFireEnd(FInputActionValue());
        // 재장전 캔슬
        OwnerCharacter->FireComp->CancelReload();
        // 조준 해제 (UI 포함)
        OwnerCharacter->FireComp->ForceEndAim();
    }

    // 스테미나 소모
    //CurrentStamina -= RollStaminaCost;
    //TimeSinceStaminaUse = 0.f;
    //OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

    // 쿨타임 시작
    bCanRoll = false;
    GetWorld()->GetTimerManager().SetTimer(
        RollCooldownHandle,
        this,
        &UAZPlayerMoveComponent::ResetRollCooldown,
        RollCooldown,
        false
    );


    FVector2D Dir2D = CurrentMoveInput.IsNearlyZero() ? FVector2D(0, 1) : CurrentMoveInput;
    FVector Dir3D = FVector(Dir2D.Y, Dir2D.X, 0.f).GetSafeNormal();

    FRotator ControlRot(0.f, OwnerCharacter->GetControlRotation().Yaw, 0.f);
    FVector RollDir = ControlRot.RotateVector(Dir3D);

    Server_Roll(RollDir);
}

void UAZPlayerMoveComponent::ResetRollCooldown()
{
    bCanRoll = true;
}

void UAZPlayerMoveComponent::Server_Roll_Implementation(FVector  RollDirection)
{
    CurrentStamina = FMath::Max(0.f, CurrentStamina - RollStaminaCost);
    TimeSinceStaminaUse = 0.f;
    OnRep_Stamina();
    Multicast_Roll(RollDirection);
}

void UAZPlayerMoveComponent::Multicast_Roll_Implementation(FVector RollDirection)
{
    if (bIsRolling) return;

    bIsRolling = true;
    RollElapsed = 0.f;

    // 무적 시작
    StartInvincible(InvincibleTime);

    if (OwnerCharacter)
    {
        OwnerCharacter->bUseControllerRotationYaw = false;

    }

    FRotator RollRotation = RollDirection.Rotation();
    OwnerCharacter->SetActorRotation(FRotator(0.f, RollRotation.Yaw, 0.f));

    RollStartLocation = OwnerCharacter->GetActorLocation();
    RollTargetLocation = RollStartLocation + RollDirection * RollDistance;

    UE_LOG(LogTemp, Warning, TEXT("RollDir: %s"), *RollDirection.ToString());
    UE_LOG(LogTemp, Warning, TEXT("New Rotation: %.2f"), RollRotation.Yaw);

    if (RollVFX && OwnerCharacter)
    {
        RollVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            RollVFX,
            OwnerCharacter->GetMesh(),
            RollVFXSocket, 
            RollVFXOffset,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            false
        );
    }

    if (UAZPlayerAnimInstance* Anim = Cast<UAZPlayerAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
    {
        Anim->PlayRollAnim();
    }

    // 이동 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(
        RollTimerHandle,
        this,
        &UAZPlayerMoveComponent::HandleRolling,
        0.01f,
        true
    );
}

void UAZPlayerMoveComponent::HandleRolling()
{
    RollElapsed += 0.01f;
    float Alpha = FMath::Clamp(RollElapsed / RollDuration, 0.f, 1.f);

    float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, RollEaseExponent);

    FVector NewLocation = FMath::Lerp(RollStartLocation, RollTargetLocation, EasedAlpha);
    OwnerCharacter->SetActorLocation(NewLocation, true);

    if (Alpha >= 1.0f)
    {
        GetWorld()->GetTimerManager().ClearTimer(RollTimerHandle);
        RollStartRotation = OwnerCharacter->GetActorRotation();
        RollTargetRotation = OwnerCharacter->GetControlRotation();
        RecoveryElapsed = 0.f;
        bIsRecoveringFromRoll = true;
    }
}

void UAZPlayerMoveComponent::EndRoll()
{
    bIsRolling = false;
    bIsRecoveringFromRoll = false;
    RollElapsed = 0.f;
    RecoveryElapsed = 0.f;

    GetWorld()->GetTimerManager().ClearTimer(RollTimerHandle);
    if (OwnerCharacter)
    {
        OwnerCharacter->bUseControllerRotationYaw = true;
        if (OwnerCharacter->FireComp)
        {
            OwnerCharacter->FireComp->AimOffset_StartYaw = OwnerCharacter->GetControlRotation().Yaw;
            OwnerCharacter->FireComp->AimOffset_Yaw = 0.f;
        }

    }


    // VFX 종료
    if (RollVFXComponent)
    {
        RollVFXComponent->DestroyComponent();
        RollVFXComponent = nullptr;
    }
}

void UAZPlayerMoveComponent::StartInvincible(float Duration)
{
    if (!OwnerCharacter || !OwnerCharacter->HealthComp) return;

    OwnerCharacter->HealthComp->SetInvincible(true);

    GetWorld()->GetTimerManager().ClearTimer(InvincibleTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(
        InvincibleTimerHandle,
        [this]()
        {
            if (OwnerCharacter && OwnerCharacter->HealthComp)
            {
                OwnerCharacter->HealthComp->SetInvincible(false);
            }
        },
        Duration,
        false
    );
}

// ==================== 속도 업데이트 ====================

void UAZPlayerMoveComponent::UpdateSpeed_Server_Implementation(float PercentBonus, float PercentPenalty)
{
    float BagPenalty = 1.0f - PercentPenalty;
    WalkSpeed = (BaseWalkSpeed * (1.0f + PercentBonus)) * BagPenalty;
    RunSpeed = (BaseRunSpeed * (1.0f + PercentBonus)) * BagPenalty;
    UE_LOG(LogTemp, Warning, TEXT("WalkSpeed: %f, RunSpeed: %f"), WalkSpeed, RunSpeed);
    OnRep_UpdateSpeed();
}

// ==================== 스테미나 ====================

void UAZPlayerMoveComponent::UpdateStamina(float DeltaTime)
{
    if (!OwnerCharacter || OwnerCharacter->GetLocalRole() == ROLE_SimulatedProxy)
        return;

    bool bIsMoving = !CurrentMoveInput.IsNearlyZero();
    if (OwnerCharacter->HasAuthority())
    {
        bIsMoving = OwnerCharacter->GetVelocity().SizeSquared() > 10.f;
    }

    if (bIsRun && bIsMoving)
    {
        CurrentStamina = FMath::Max(0.f, CurrentStamina - StaminaDrainRate * DeltaTime);
        TimeSinceStaminaUse = 0.f;

        if (CurrentStamina <= 0.f)
        {
            bIsRun = false;
            Server_SprintEnd();
        }

        OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
    }
    else
    {
        // 회복 딜레이 후 스테미나 회복
        TimeSinceStaminaUse += DeltaTime;

        if (TimeSinceStaminaUse >= StaminaRegenDelay && CurrentStamina < MaxStamina)
        {
            CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
            OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
        }
    }
}

void UAZPlayerMoveComponent::SetFreeLookMode(bool bEnable)
{
    bIsFreeLookMode = bEnable;

    if (!OwnerCharacter) return;

    if (bEnable)
    {
        // 마을 모드 
        OwnerCharacter->bUseControllerRotationYaw = false;

        if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
        {
            MoveComp->bOrientRotationToMovement = true;
            MoveComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
        }

        // 무기 숨기기
        if (OwnerCharacter->FireComp && OwnerCharacter->FireComp->CurrentWeapon)
        {
            OwnerCharacter->FireComp->CurrentWeapon->SetActorHiddenInGame(true);
        }
    }
    else
    {
        // 인게임 모드 
        OwnerCharacter->bUseControllerRotationYaw = true;

        if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
        {
            MoveComp->bOrientRotationToMovement = false;
        }

        // 무기 보이기
        if (OwnerCharacter->FireComp && OwnerCharacter->FireComp->CurrentWeapon)
        {
            OwnerCharacter->FireComp->CurrentWeapon->SetActorHiddenInGame(false);
        }
    }
    if (OwnerCharacter->FireComp)
    {
        OwnerCharacter->FireComp->SetFreeLookMode(bEnable);
    }
}