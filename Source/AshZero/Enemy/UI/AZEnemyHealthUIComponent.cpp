// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/UI/AZEnemyHealthUIComponent.h"
#include "Enemy/UI/AZEnemyHealthBarWidget.h"
#include "Enemy/UI/AZBossHealthBarWidget.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Enemy/AZEnemyBase.h"  
#include "Components/AZHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

UAZEnemyHealthUIComponent::UAZEnemyHealthUIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

}

void UAZEnemyHealthUIComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!bIsBoss)
    {
        SetupEnemyUI();
    }
    if (AAZEnemyBase* Enemy = Cast<AAZEnemyBase>(GetOwner()))
    {
        if (UAZHealthComponent* HealthComp = Enemy->GetHealthComponent())
        {
            HealthComp->OnHealthChanged.AddDynamic(this, &UAZEnemyHealthUIComponent::OnHealthChanged);
        }
    }
}

void UAZEnemyHealthUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (BossWidget && BossWidget->IsInViewport())
    {
        BossWidget->RemoveFromParent();
        BossWidget = nullptr;
    }
}

void UAZEnemyHealthUIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 보스는 HUD라서 스킵
    if (bIsBoss) return;
    if (!HealthWidgetComp || !HealthWidgetComp->IsVisible()) return;
    if (!bScaleByDistance) return;

    // 플레이어와의 거리 계산
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PC->GetPawn()->GetActorLocation());

    // 최대 거리 넘으면 숨김
    if (Distance > MaxDrawDistance)
    {
        HealthWidgetComp->SetVisibility(false);
        return;
    }

    // 거리 기반 스케일 계산
    float Alpha = FMath::Clamp((Distance - MinDrawDistance) / (MaxDrawDistance - MinDrawDistance), 0.f, 1.f);
    float Scale = FMath::Lerp(1.0f, MinScale, Alpha);

    // 스케일 적용
    HealthWidgetComp->SetDrawSize(HealthBarSize * Scale);
}

void UAZEnemyHealthUIComponent::SetupEnemyUI()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SetupEnemyUI START ==="));

    if (!EnemyWidgetClass || !GetOwner()) return;

    // 1. WidgetComponent 생성
    HealthWidgetComp = NewObject<UWidgetComponent>(GetOwner(), TEXT("HealthWidgetComp"));
    if (!HealthWidgetComp) return;

    HealthWidgetComp->RegisterComponent();

    HealthWidgetComp->AttachToComponent(
        GetOwner()->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform
    );

    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        float CapsuleHalfHeight = OwnerChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        HealthWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, CapsuleHalfHeight + FixedHeightOffset));
    }
    else
    {
        HealthWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, FixedHeightOffset));
    }


    // 3. 위젯 설정
    HealthWidgetComp->SetWidgetClass(EnemyWidgetClass);
    HealthWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
    HealthWidgetComp->SetDrawSize(HealthBarSize);
    HealthWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HealthWidgetComp->SetVisibility(false);

    HealthWidgetComp->InitWidget();
    EnemyWidget = Cast<UAZEnemyHealthBarWidget>(HealthWidgetComp->GetWidget());
}

void UAZEnemyHealthUIComponent::SetupBossUI()
{
    if (!BossWidgetClass) return;
    
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // 이미 존재하면 리턴
    if (BossWidget && BossWidget->IsInViewport()) return;

    // 보스 위젯 생성 (플레이어 HUD에 추가)
    BossWidget = CreateWidget<UAZBossHealthBarWidget>(PC, BossWidgetClass);
    if (BossWidget)
    {
        BossWidget->AddToViewport(100);  // 높은 Z-Order
        float InitialHp = 0.0f;
        float InitialMaxHp = 0.0f;

        if (AAZEnemyBase* Enemy = Cast<AAZEnemyBase>(GetOwner()))
        {
            if (UAZHealthComponent* HealthComp = Enemy->GetHealthComponent())
            {
                InitialHp = HealthComp->GetHp();
                InitialMaxHp = HealthComp->GetMaxHp();
            }
        }

        BossWidget->InitBoss(BossName, InitialMaxHp);
        BossWidget->UpdateHealth(InitialHp, InitialMaxHp);  // ★ 초기 HP 반영 ★
        BossWidget->ShowBossHealthBar();

    }
}

void UAZEnemyHealthUIComponent::OnHealthChanged(float CurrentHealth, float MaxHealth)
{
    CachedMaxHealth = MaxHealth;

    if (bIsBoss)
    {
        if (GetOwner()->HasAuthority())
        {
            MulticastUpdateBossHealth(CurrentHealth, MaxHealth);
        }
        if (BossWidget)
        {
            BossWidget->UpdateHealth(CurrentHealth, MaxHealth);
        }
    }
    else
    {
        // 일반 몬스터는 기존 로직
        if (EnemyWidget)
        {
            EnemyWidget->UpdateHealth(CurrentHealth, MaxHealth);
            EnemyWidget->ShowHealthBar();
        }

        if (HealthWidgetComp)
        {
            HealthWidgetComp->SetVisibility(true);
        }
    }
}

void UAZEnemyHealthUIComponent::MulticastUpdateBossHealth_Implementation(float CurrentHealth, float MaxHealth)
{
    // 모든 클라이언트에서 보스 HP 바 업데이트
    if (BossWidget)
    {
        BossWidget->UpdateHealth(CurrentHealth, MaxHealth);
    }
}


void UAZEnemyHealthUIComponent::ActivateBossUI()
{
    UE_LOG(LogTemp, Warning, TEXT("ActivateBossUI called! HasAuthority: %s, bIsBoss: %s"),
        GetOwner()->HasAuthority() ? TEXT("YES") : TEXT("NO"),
        bIsBoss ? TEXT("YES") : TEXT("NO"));

    if (GetOwner()->HasAuthority() && bIsBoss)
    {
        UE_LOG(LogTemp, Warning, TEXT("Calling MulticastShowBossUI!"));
        MulticastShowBossUI();
    }
}

void UAZEnemyHealthUIComponent::MulticastShowBossUI_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("MulticastShowBossUI_Implementation called!"));

    if (bIsBoss)
    {
        SetupBossUI();
    }
}

void UAZEnemyHealthUIComponent::OnBossDefeated()
{
    if (BossWidget)
    {
        BossWidget->PlayDefeatAnimation();
    }
}

void UAZEnemyHealthUIComponent::MulticastBossDefeated_Implementation()
{
    if (BossWidget)
    {
        BossWidget->PlayDefeatAnimation();
    }
}
void UAZEnemyHealthUIComponent::MulticastHideBossUI_Implementation()
{
    if (BossWidget)
    {
        BossWidget->HideBossHealthBar();
    }
}


void UAZEnemyHealthUIComponent::OnDeath()
{
    if (bIsBoss && BossWidget)
    {
        BossWidget->PlayDefeatAnimation();
    }
    else if (EnemyWidget)
    {
        EnemyWidget->HideHealthBar();
    }
}

void UAZEnemyHealthUIComponent::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
    OnHealthChanged(CurrentHealth, MaxHealth);
}
 