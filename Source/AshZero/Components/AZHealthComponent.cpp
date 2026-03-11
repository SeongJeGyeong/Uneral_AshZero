
#include "Components/AZHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/AZPlayerCharacter.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GenericTeamAgentInterface.h"
#include "AshZero.h"

UAZHealthComponent::UAZHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UAZHealthComponent::BeginPlay()
{
    Super::BeginPlay();

	MaxHp = BaseMaxHp;  //최대 체력 설정 , BaseMaxHp 만 BP에서 조정
    Hp = MaxHp;
    CurrentFullness = MaxFullness;
    CurrentHydration = MaxHydration;
    CurrentMaxHpMultiplier = 1.0f;

	// 초기 값 Broadcast
    OnHealthChanged.Broadcast(Hp, MaxHp);
    OnSurvivalStatsChanged.Broadcast(CurrentFullness, MaxFullness, CurrentHydration, MaxHydration);

    // Damage 이벤트 바인딩 (서버만)
    if(GetOwnerRole() == ROLE_Authority)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            Owner->OnTakeAnyDamage.AddDynamic(this, &UAZHealthComponent::OnTakeAnyDamageCallback);
        }
        // 1초마다 생존 상태 업데이트 타이머 가동
        GetWorld()->GetTimerManager().SetTimer(SurvivalTimerHandle, this, &UAZHealthComponent::HandleSurvivalUpdate, 1.0f, true);
    }
}
void UAZHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAZHealthComponent, Hp);
    DOREPLIFETIME(UAZHealthComponent, MaxHp);
    DOREPLIFETIME(UAZHealthComponent, CurrentFullness);
    DOREPLIFETIME(UAZHealthComponent, CurrentHydration);

}
void UAZHealthComponent::HandleSurvivalUpdate()
{
    if (GetOwnerRole() != ROLE_Authority || bIsDead) return;

    FullnessElapsed += 1.0f;
    HydrationElapsed += 1.0f;

    // 허기 감소 로직
    if (FullnessElapsed >= FullnessTickInterval)
    {
        CurrentFullness = FMath::Max(0.f, CurrentFullness - FullnessDecreaseAmount);
        FullnessElapsed = 0.f;
    }

    // 갈증 감소 로직
    if (HydrationElapsed >= HydrationTickInterval)
    {
        CurrentHydration = FMath::Max(0.f, CurrentHydration - HydrationDecreaseAmount);
        HydrationElapsed = 0.f;
    }

    // 값 변경 방송 (서버측 UI 및 수치 동기화용)
    OnSurvivalStatsChanged.Broadcast(CurrentFullness, MaxFullness, CurrentHydration, MaxHydration);
    // 2. 플레이어에게 상태 이상 전달
    AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(GetOwner());
    if (Player)
    {
        float FullnessP = (CurrentFullness / MaxFullness) * 100.f;
        float HydrationP = (CurrentHydration / MaxHydration) * 100.f;

        // 허기 단계별 체크 
        bool bFoundFullnessEffect = false;
        for (const FSurvivalThreshold& Step : FullnessThresholds)
        {
            if (FullnessP <= Step.ThresholdPercent)
            {
                Player->ApplyStatusEffect(Step.Effect);
                bFoundFullnessEffect = true;
                break; // 가장 우선순위 높은(가장 배고픈) 효과 하나만 적용
            }
        }
        // 임계값을 벗어났으면 (포만도가 충분하면) 효과 제거
        if (!bFoundFullnessEffect)
        {
            // Starvation 디버프 해제 신호 전송
            FAZStatusEffectPacket ClearPacket;
            ClearPacket.StatusType = EAZStatusType::Starvation;
            ClearPacket.StatMultiplier = 1.0f;  // 1.0 = 정상
            ClearPacket.Duration = 0.0f;
            Player->ApplyStatusEffect(ClearPacket);
        }


        // 갈증 단계별 체크
        bool bFoundHydrationEffect = false;
        for (const FSurvivalThreshold& Step : HydrationThresholds)
        {
            if (HydrationP <= Step.ThresholdPercent)
            {
                Player->ApplyStatusEffect(Step.Effect);
                bFoundHydrationEffect = true;
                break;
            }
        }
        // 임계값을 벗어났으면 DoT 직접 해제
        if (!bFoundHydrationEffect)
        {
            Player->StopDoTEffect(EAZStatusType::Dehydration);
        }
    }
}

void UAZHealthComponent::SetInvincible(bool bNewInvincible)
{
    bIsInvincible = bNewInvincible;
}

void UAZHealthComponent::SetMaxHpMultiplier(float NewMultiplier)
{
    if (GetOwnerRole() != ROLE_Authority) return;

    CurrentMaxHpMultiplier = NewMultiplier;

    float OldMaxHp = MaxHp;
    MaxHp = BaseMaxHp * CurrentMaxHpMultiplier;

    // 현재 체력이 새 최대치를 초과하면 조정
    if (Hp > MaxHp)
    {
        Hp = MaxHp;
    }

    OnHealthChanged.Broadcast(Hp, MaxHp);
}

void UAZHealthComponent::ResetMaxHpMultiplier()
{
    SetMaxHpMultiplier(1.0);
}

void UAZHealthComponent::OnRep_Hp()
{
    UE_LOG(LogTemp, Warning, TEXT("[Client] OnRep_Hp: %.1f / %.1f"), Hp, MaxHp);
    OnHealthChanged.Broadcast(Hp, MaxHp);
}

void UAZHealthComponent::OnRep_SurvivalStats()
{
    OnSurvivalStatsChanged.Broadcast(CurrentFullness, MaxFullness, CurrentHydration, MaxHydration);
}

void UAZHealthComponent::OnTakeAnyDamageCallback(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    UE_LOG(LogTemp, Error, TEXT("[HealthComp] OnTakeAnyDamage - Damage: %.1f, Auth: %s, Dead: %s, Invincible: %s"),
        Damage,
        GetOwnerRole() == ROLE_Authority ? TEXT("YES") : TEXT("NO"),
        bIsDead ? TEXT("YES") : TEXT("NO"),
        bIsInvincible ? TEXT("YES") : TEXT("NO"));

    if (GetOwnerRole() != ROLE_Authority || bIsDead || bIsInvincible) return;

    // 팀 체크
    if (InstigatedBy)
    {
        APawn* InstigatorPawn = InstigatedBy->GetPawn();
        IGenericTeamAgentInterface* InstigatorTeam = Cast<IGenericTeamAgentInterface>(InstigatorPawn);
        IGenericTeamAgentInterface* MyTeam = Cast<IGenericTeamAgentInterface>(GetOwner());

        if (InstigatorTeam && MyTeam)
        {
            if (InstigatorTeam->GetGenericTeamId() == MyTeam->GetGenericTeamId())
            {
                // 같은 팀 - 데미지 무시
                UE_LOG(LogTemp, Warning, TEXT("[Health] Friendly fire blocked!"));
                return;
            }
        }
    }

    Hp = FMath::Clamp(Hp - Damage, 0.f, MaxHp);

    OnHealthChanged.Broadcast(Hp, MaxHp);
    OnHitRegistered.Broadcast(Damage, DamageCauser, GetOwner()->GetActorLocation());
    if (Hp <= 0.f)
    {
        MulticastDeath(DamageCauser);
    }
}

void UAZHealthComponent::MulticastDeath_Implementation(AActor* Killer)
{
    if (bIsDead) return;
    bIsDead = true;

    // 사망 델리게이트 호출 (데스박스 생성용)
    AActor* Owner = GetOwner();
    if (Owner)
    {
        OnDeath.Broadcast(Owner->GetActorLocation(), Killer);
    }
}

void UAZHealthComponent::SetHealth(float NewHealth)
{
    if (GetOwner()->HasAuthority())
    {
        Hp = FMath::Clamp(NewHealth, 0.f, MaxHp);
        OnHealthChanged.Broadcast(Hp, MaxHp);
    }
}

void UAZHealthComponent::AddHealth(float HealthValue)
{
    AddHealth_Sever(HealthValue);
}

void UAZHealthComponent::AddHealth_Sever_Implementation(float HealthValue)
{
    if (bIsDead) return;
    Hp = FMath::Clamp(Hp + HealthValue, 0.f, MaxHp);
    OnHealthChanged.Broadcast(Hp, MaxHp);
    UE_LOG(LogTemp, Warning, TEXT("HP : %f, MaxHP: %f"), Hp, MaxHp);
}

void UAZHealthComponent::AddHydration(float HydrationthValue)
{
    AddHydration_Sever(HydrationthValue);
}

void UAZHealthComponent::AddHydration_Sever_Implementation(float HydrationthValue)
{
    if (bIsDead) return;
    CurrentHydration = FMath::Clamp(CurrentHydration + HydrationthValue, 0.f, MaxHydration);
    OnSurvivalStatsChanged.Broadcast(CurrentFullness, MaxFullness, CurrentHydration, MaxHydration);
}

void UAZHealthComponent::AddFullness(float FullnessValue)
{
    AddFullness_Sever(FullnessValue);
}

void UAZHealthComponent::AddFullness_Sever_Implementation(float FullnessValue)
{
    if (bIsDead) return;
    CurrentFullness = FMath::Clamp(CurrentFullness + FullnessValue, 0.f, MaxFullness);
    OnSurvivalStatsChanged.Broadcast(CurrentFullness, MaxFullness, CurrentHydration, MaxHydration);
}

void UAZHealthComponent::ApplyStatusEffect(const FAZStatusEffectPacket& Packet, AActor* Causer)
{
    if (!GetOwner()->HasAuthority() || Hp <= 0.0f) return;

    EAZStatusType Type = Packet.StatusType;

    // 이미 걸려있다면 기존 타이머 제거하고 새로 갱신
    if (ActiveDoTEffects.Contains(Type))
    {
        StopStatusEffect(Type);
    }

    // DoT 효과라면 타이머 시작
    if (IsHealthDoT(Type))
    {
        FActiveDoTEffect NewEffect;
        NewEffect.EffectPacket = Packet;
        NewEffect.RemainingDuration = Packet.Duration;

        FTimerDelegate TimerDel;
        TimerDel.BindUObject(this, &UAZHealthComponent::ProcessDoTTick, Type, Causer);

        GetWorld()->GetTimerManager().SetTimer(
            NewEffect.TickTimerHandle,
            TimerDel,
            Packet.TickInterval,
            true
        );

        ActiveDoTEffects.Add(Type, NewEffect);
    }
}

void UAZHealthComponent::ProcessDoTTick(EAZStatusType StatusType, AActor* Causer)
{
    if (!ActiveDoTEffects.Contains(StatusType)) return;
    FActiveDoTEffect& Effect = ActiveDoTEffects[StatusType];

    // 1. 데미지 적용
    if (Effect.EffectPacket.ValuePerTick > 0.0f)
    {
        UGameplayStatics::ApplyDamage(
            GetOwner(),
            Effect.EffectPacket.ValuePerTick,
            Causer ? Causer->GetInstigatorController() : nullptr,
            Causer,
            nullptr
        );
    }

    // 2. 시간 관리
    if (Effect.EffectPacket.Duration > 0.0f)
    {
        Effect.RemainingDuration -= Effect.EffectPacket.TickInterval;
        if (Effect.RemainingDuration <= 0.0f)
        {
            StopStatusEffect(StatusType);
        }
    }
}

void UAZHealthComponent::StopStatusEffect(EAZStatusType StatusType)
{
    if (ActiveDoTEffects.Contains(StatusType))
    {
        GetWorld()->GetTimerManager().ClearTimer(ActiveDoTEffects[StatusType].TickTimerHandle);
        ActiveDoTEffects.Remove(StatusType);
    }
}

bool UAZHealthComponent::IsHealthDoT(EAZStatusType Type) const
{
    switch (Type)
    {
    case EAZStatusType::Poison:
    case EAZStatusType::Bleeding:
    case EAZStatusType::Fire:
    case EAZStatusType::Starvation:
    case EAZStatusType::Dehydration:
        return true;
    }
    return false;
}