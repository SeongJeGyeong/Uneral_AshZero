// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AZStatusBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Weapon/AZWeapon.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/AZHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "System/Player/AZPlayerController.h"
#include "Components/AZEquipmentComponent.h"
#include "UI/HUD/AZQuickSlotWidget.h"
#include "UI/HUD/AZHUD.h"
#include "Item/AZItemBase.h"
#include "GameFramework/PlayerState.h"
#include "Components/AZPlayerMoveComponent.h"

void UAZStatusBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 팀원 슬롯 초기에 숨기기
    HideTeamMemberSlot(0);
    HideTeamMemberSlot(1);

    // 배열 초기화
    LinkedTeamMembers.SetNum(2);

    AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwningPlayer());
    if (PC == nullptr) return;

    if (UAZEquipmentComponent* EquipmentComponent = PC->GetComponentByClass<UAZEquipmentComponent>())
    {
        EquipmentComponent->OnEquipmentUpdated.AddDynamic(this, &UAZStatusBarWidget::OnEquipmentChanged);
    }

    if (AAZHUD* HUD = Cast<AAZHUD>(PC->GetHUD()))
    {
        HUD->StatusBarWidget = this;
    }

    //BindToTargetCharacter(Cast<AAZPlayerCharacter>(PC->GetPawn()));
    
}
void UAZStatusBarWidget::NativeDestruct()
{
    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TeamCheckTimerHandle);
    }

    // 현재 타겟 바인딩 해제
    if (CurrentTarget.IsValid())
    {
        BindHealthDelegates(CurrentTarget.Get(), false);
    }

    // 팀원 바인딩 해제
    for (int32 i = 0; i < LinkedTeamMembers.Num(); i++)
    {
        if (LinkedTeamMembers[i])
        {
            UnbindTeamMemberHealth(i, LinkedTeamMembers[i]);
        }
    }

    Super::NativeDestruct();
}
void UAZStatusBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsGhostDecreasing && GhostHealthProgressBar)
    {
        float CurrentGhostPercent = GhostHealthProgressBar->GetPercent();
        float NewGhostPercent = FMath::FInterpConstantTo(CurrentGhostPercent, TargetHealthPercent, InDeltaTime, GhostDecreaseRate);
        GhostHealthProgressBar->SetPercent(NewGhostPercent);
        if (FMath::IsNearlyEqual(NewGhostPercent, TargetHealthPercent, 0.001f))
        {
            bIsGhostDecreasing = false;
            GhostHealthProgressBar->SetPercent(TargetHealthPercent); // 값 보정
        }
    }
}
void UAZStatusBarWidget::UpdateHealth(float Current, float Max)
{
    float NewPercent = (Max > 0.f) ? (Current / Max) : 0.f;
    if (HealthBar)
    {
        HealthBar->SetPercent(NewPercent);
    }

    TargetHealthPercent = NewPercent;
    if (GhostHealthProgressBar)
    {
        float CurrentGhostPercent = GhostHealthProgressBar->GetPercent();

        if (NewPercent < CurrentGhostPercent)
        {
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().ClearTimer(GhostDelayTimerHandle);
                bIsGhostDecreasing = false;

                GetWorld()->GetTimerManager().SetTimer(GhostDelayTimerHandle, this, &UAZStatusBarWidget::StartGhostDecrease, 1.0f, false);
            }
        }
        else
        {
            GhostHealthProgressBar->SetPercent(NewPercent);
            bIsGhostDecreasing = false;
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().ClearTimer(GhostDelayTimerHandle);
            }
        }
    }
}

void UAZStatusBarWidget::StartGhostDecrease()
{
    if (GhostHealthProgressBar && HealthBar)
    {
        float CurrentGhost = GhostHealthProgressBar->GetPercent();
        float TargetHealth = HealthBar->GetPercent();
        float Gap = CurrentGhost - TargetHealth;
        if (Gap > 0.f)
        {
            GhostDecreaseRate = Gap / 1.0f;
            bIsGhostDecreasing = true;
        }
    }
}

void UAZStatusBarWidget::UpdateStamina(float Current, float Max)
{
    if (StaminaBar)
    {
        float Percent = Max > 0.f ? Current / Max : 0.f;
        StaminaBar->SetPercent(Percent);
    }
}

void UAZStatusBarWidget::UpdateWeapon(AAZWeapon* CurrentWeapon, AAZWeapon* SubWeaponRef, int32 CurrentSlot)
{
    // 메인 무기 아이콘
    if (MainWeapon && CurrentWeapon && CurrentWeapon->WeaponIcon)
    {
        MainWeapon->SetBrushFromTexture(CurrentWeapon->WeaponIcon);
        MainWeapon->SetVisibility(ESlateVisibility::HitTestInvisible);
        WeaponSlotOutline1->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else if (MainWeapon)
    {
        MainWeapon->SetVisibility(ESlateVisibility::Hidden);
        WeaponSlotOutline1->SetVisibility(ESlateVisibility::Hidden);
    }

    // 서브 무기 아이콘
    if (SubWeapon && SubWeaponRef && SubWeaponRef->WeaponIcon)
    {
        SubWeapon->SetBrushFromTexture(SubWeaponRef->WeaponIcon);
        SubWeapon->SetVisibility(ESlateVisibility::HitTestInvisible);
        WeaponSlotOutline2->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else if (SubWeapon)
    {
        SubWeapon->SetVisibility(ESlateVisibility::Hidden);
        WeaponSlotOutline2->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}

void UAZStatusBarWidget::UpdateAmmo(int32 CurrentAmmo, int32 MaxAmmo)
{
    if (CurrentAmmoText)
    {
        CurrentAmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentAmmo)));
    }

    if (MaxAmmoText)
    {
        MaxAmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d"), MaxAmmo)));
    }
}

// 팀원 UI 초기화
void UAZStatusBarWidget::InitializeTeamMemberUI(AAZPlayerCharacter* LocalPlayer)
{
    OwningPlayer = LocalPlayer;

    BindToTargetCharacter(LocalPlayer);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TeamCheckTimerHandle,
            this,
            &UAZStatusBarWidget::CheckForTeamMembers,
            1.0f,
            true
        );
    }
}

void UAZStatusBarWidget::BindToTargetCharacter(AAZPlayerCharacter* NewTargetCharacter)
{
    if (CurrentTarget.IsValid())
    {
        BindHealthDelegates(CurrentTarget.Get(), false);
    }

    CurrentTarget = NewTargetCharacter;

    if (CurrentTarget.IsValid())
    {
        BindHealthDelegates(CurrentTarget.Get(), true);

        // 초기값 업데이트
        if (UAZHealthComponent* HealthComp = CurrentTarget->GetComponentByClass<UAZHealthComponent>())
        {
            UpdateHealth(HealthComp->Hp, HealthComp->MaxHp);
        }
        if (UAZPlayerMoveComponent* MoveComp = CurrentTarget->GetComponentByClass<UAZPlayerMoveComponent>())
        {
            UpdateStamina(MoveComp->CurrentStamina, MoveComp->MaxStamina);
        }
    }
    else
    {
        UpdateHealth(0.f, 100.f);
        UpdateStamina(0.f, 100.f);
    }
}

void UAZStatusBarWidget::BindHealthDelegates(AAZPlayerCharacter* Target, bool bBind)
{
    if (!Target) return;
    UAZHealthComponent* HealthComp = Target->GetComponentByClass<UAZHealthComponent>();
    UAZPlayerMoveComponent* MoveComp = Target->GetComponentByClass<UAZPlayerMoveComponent>();

    if (bBind)
    {
        if (HealthComp)
        {
            HealthComp->OnHealthChanged.RemoveDynamic(this, &UAZStatusBarWidget::UpdateHealth);
            HealthComp->OnHealthChanged.AddDynamic(this, &UAZStatusBarWidget::UpdateHealth);
        }
        if (MoveComp)
        {
            MoveComp->OnStaminaChanged.RemoveDynamic(this, &UAZStatusBarWidget::UpdateStamina);
            MoveComp->OnStaminaChanged.AddDynamic(this, &UAZStatusBarWidget::UpdateStamina);
        }
    }
    else
    {
        if (HealthComp)
        {
            HealthComp->OnHealthChanged.RemoveDynamic(this, &UAZStatusBarWidget::UpdateHealth);
        }
        if (MoveComp)
        {
            MoveComp->OnStaminaChanged.RemoveDynamic(this, &UAZStatusBarWidget::UpdateStamina);
        }
    }
}

void UAZStatusBarWidget::CheckForTeamMembers()
{
    if (!OwningPlayer) return;

    // 월드의 모든 PlayerCharacter 찾기
    TArray<AActor*> FoundPlayers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAZPlayerCharacter::StaticClass(), FoundPlayers);

    int32 SlotIndex = 0;

    for (AActor* Actor : FoundPlayers)
    {
        AAZPlayerCharacter* PlayerChar = Cast<AAZPlayerCharacter>(Actor);

        // 자기 자신 제외
        if (!PlayerChar || PlayerChar == OwningPlayer) continue;

        // 슬롯 3개까지만
        if (SlotIndex >= 2)
            break;

        // 이미 등록된 팀원인지 확인
        bool bAlreadyLinked = LinkedTeamMembers.Contains(PlayerChar);

        if (!bAlreadyLinked)
        {
            SetupTeamMemberSlot(SlotIndex, PlayerChar);
        }

        SlotIndex++;
    }

    // 나간 팀원 슬롯 숨기기
    for (int32 i = SlotIndex; i < 2; i++)
    {
        if (LinkedTeamMembers.IsValidIndex(i) && LinkedTeamMembers[i] != nullptr)
        {
            HideTeamMemberSlot(i);
            LinkedTeamMembers[i] = nullptr;
        }
    }
}

void UAZStatusBarWidget::SetupTeamMemberSlot(int32 SlotIndex, AAZPlayerCharacter* TeamMember)
{
    if (!TeamMember || SlotIndex < 0 || SlotIndex >= 2)
        return;

    if (LinkedTeamMembers.IsValidIndex(SlotIndex) && LinkedTeamMembers[SlotIndex])
    {
        UnbindTeamMemberHealth(SlotIndex, LinkedTeamMembers[SlotIndex]);
    }

    LinkedTeamMembers[SlotIndex] = TeamMember;

    FString PlayerNameStr = TEXT("Unknown");
    if (TeamMember->GetPlayerState())
    {
        PlayerNameStr = TeamMember->GetPlayerState()->GetPlayerName();
    }
    else
    {
        PlayerNameStr = TeamMember->GetName();
    }

    // 이름 설정 및 패널 표시
    switch (SlotIndex)
    {
    case 0:
        if (TeamMember1_Panel)
            TeamMember1_Panel->SetVisibility(ESlateVisibility::HitTestInvisible);
        if (TeamMember1_Name)
            TeamMember1_Name->SetText(FText::FromString(PlayerNameStr));
        if (TeamMember->HealthComp)
        {
            // ★ 중복 방지: Remove 후 Add
            TeamMember->HealthComp->OnHealthChanged.RemoveDynamic(this, &UAZStatusBarWidget::UpdateTeamMember1Health);
            TeamMember->HealthComp->OnHealthChanged.AddDynamic(this, &UAZStatusBarWidget::UpdateTeamMember1Health);
            UpdateTeamMember1Health(TeamMember->HealthComp->Hp, TeamMember->HealthComp->MaxHp);
        }
        break;

    case 1:
        if (TeamMember2_Panel)
            TeamMember2_Panel->SetVisibility(ESlateVisibility::HitTestInvisible);
        if (TeamMember2_Name)
            TeamMember2_Name->SetText(FText::FromString(PlayerNameStr));
        if (TeamMember->HealthComp)
        {
            // ★ 중복 방지: Remove 후 Add
            TeamMember->HealthComp->OnHealthChanged.RemoveDynamic(this, &UAZStatusBarWidget::UpdateTeamMember2Health);
            TeamMember->HealthComp->OnHealthChanged.AddDynamic(this, &UAZStatusBarWidget::UpdateTeamMember2Health);
            UpdateTeamMember2Health(TeamMember->HealthComp->Hp, TeamMember->HealthComp->MaxHp);
        }
        break;
    }

    UE_LOG(LogTemp, Warning, TEXT("[StatusBar] Team member %d: %s"), SlotIndex, *TeamMember->GetName());
}

void UAZStatusBarWidget::UnbindTeamMemberHealth(int32 SlotIndex, AAZPlayerCharacter* TeamMember)
{
    if (!TeamMember || !TeamMember->HealthComp) return;

    switch (SlotIndex)
    {
    case 0:
        TeamMember->HealthComp->OnHealthChanged.RemoveDynamic(this, &UAZStatusBarWidget::UpdateTeamMember1Health);
        break;
    case 1:
        TeamMember->HealthComp->OnHealthChanged.RemoveDynamic(this, &UAZStatusBarWidget::UpdateTeamMember2Health);
        break;
    }
}

void UAZStatusBarWidget::HideTeamMemberSlot(int32 SlotIndex)
{
    switch (SlotIndex)
    {
    case 0:
        if (TeamMember1_Panel)
            TeamMember1_Panel->SetVisibility(ESlateVisibility::Hidden);
        break;
    case 1:
        if (TeamMember2_Panel)
            TeamMember2_Panel->SetVisibility(ESlateVisibility::Hidden);
        break;
    }
}

void UAZStatusBarWidget::UpdateTeamMember1Health(float Current, float Max)
{
    if (TeamMember1_HealthBar)
    {
        float Percent = Max > 0.f ? Current / Max : 0.f;
        TeamMember1_HealthBar->SetPercent(Percent);
    }
}

void UAZStatusBarWidget::UpdateTeamMember2Health(float Current, float Max)
{
    if (TeamMember2_HealthBar)
    {
        float Percent = Max > 0.f ? Current / Max : 0.f;
        TeamMember2_HealthBar->SetPercent(Percent);
    }
}

void UAZStatusBarWidget::OnEquipmentChanged(EEquipmentSlot SlotType, ESlotIndex SlotIndex, UAZItemBase* Item)
{
    if (SlotType != EEquipmentSlot::QuickSlot)
        return;
    UAZQuickSlotWidget* TargetWidgetSlot = nullptr;
    switch (SlotIndex)
    {
    case ESlotIndex::Slot_1: TargetWidgetSlot = QuickSlot_1; break;
    case ESlotIndex::Slot_2:	TargetWidgetSlot = QuickSlot_2; break;
    case ESlotIndex::Slot_3: TargetWidgetSlot = QuickSlot_3; break;
    case ESlotIndex::Slot_4: TargetWidgetSlot = QuickSlot_4; break;
    default: break;
    }

    if (TargetWidgetSlot == nullptr) return;

    if (Item == nullptr)
    {
        TargetWidgetSlot->ItemImage->SetVisibility(ESlateVisibility::Collapsed);
        TargetWidgetSlot->StackCount->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        UMaterialInterface* MaterialInstance = Item->GetIcon();
        if (MaterialInstance)
        {
            TargetWidgetSlot->ItemImage->SetBrushFromMaterial(Item->GetIcon());
        }
        TargetWidgetSlot->ItemImage->SetVisibility(ESlateVisibility::HitTestInvisible);
        TargetWidgetSlot->StackCount->SetText(FText::AsNumber(Item->GetStackCount()));
        TargetWidgetSlot->StackCount->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    
}