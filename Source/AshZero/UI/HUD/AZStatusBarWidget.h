// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/AZDefine.h"
#include "AZStatusBarWidget.generated.h"

/**
 * 
 */

class UProgressBar;
class UTextBlock;
class AAZWeapon;
class AAZPlayerCharacter;
class UAZQuickSlotWidget;
class UAZItemBase;
class UCanvasPanel;

UCLASS()
class ASHZERO_API UAZStatusBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ========== 체력/스테미나 ==========
    // 체력바
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> GhostHealthProgressBar;

    // 스테미나바
    UPROPERTY(meta = (BindWidget))
    UProgressBar* StaminaBar;

    UFUNCTION(BlueprintCallable, Category = "AZ|UI")
    void UpdateHealth(float Current, float Max);

    UFUNCTION(BlueprintCallable, Category = "AZ|UI")
    void UpdateStamina(float Current, float Max);

    // ========== 무기 UI ==========
    UPROPERTY(meta = (BindWidget))
    class UImage* MainWeapon;

    UPROPERTY(meta = (BindWidget))
    class UImage* SubWeapon;

    UPROPERTY(meta = (BindWidget))
    class UImage* WeaponSlotOutline1;

    UPROPERTY(meta = (BindWidget))
    class UImage* WeaponSlotOutline2;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CurrentAmmoText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* MaxAmmoText;

    UFUNCTION(BlueprintCallable)
    void UpdateWeapon(AAZWeapon* CurrentWeapon, AAZWeapon* SubWeaponRef, int32 CurrentSlot);

    UFUNCTION(BlueprintCallable)
    void UpdateAmmo(int32 CurrentAmmo, int32 MaxAmmo);

    // ========== 팀원 UI (3슬롯) ==========
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCanvasPanel> TeamPanel;
    // 팀원 1
    UPROPERTY(meta = (BindWidget))
    class UCanvasPanel* TeamMember1_Panel;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TeamMember1_Name;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* TeamMember1_HealthBar;

    // 팀원 2
    UPROPERTY(meta = (BindWidget))
    class UCanvasPanel* TeamMember2_Panel;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TeamMember2_Name;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* TeamMember2_HealthBar;

    // ========== 팀원 관리 ==========
    UFUNCTION(BlueprintCallable)
    void InitializeTeamMemberUI(AAZPlayerCharacter* LocalPlayer);

	// 타겟 캐릭터에 바인딩
    void BindToTargetCharacter(AAZPlayerCharacter* NewTargetCharacter);

    // 팀원 체력 업데이트 (인덱스별)
    UFUNCTION()
    void UpdateTeamMember1Health(float Current, float Max);

    UFUNCTION()
    void UpdateTeamMember2Health(float Current, float Max);

    // 퀵슬롯
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UAZQuickSlotWidget> QuickSlot_1;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UAZQuickSlotWidget> QuickSlot_2;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UAZQuickSlotWidget> QuickSlot_3;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UAZQuickSlotWidget> QuickSlot_4;


protected:
    void CheckForTeamMembers();
    void SetupTeamMemberSlot(int32 SlotIndex, AAZPlayerCharacter* TeamMember);
    void HideTeamMemberSlot(int32 SlotIndex);
    void UnbindTeamMemberHealth(int32 SlotIndex, AAZPlayerCharacter* TeamMember);

    FTimerHandle TeamCheckTimerHandle;

    UPROPERTY()
    AAZPlayerCharacter* OwningPlayer;

	// 연결된 팀원 캐릭터 (혹시 모를 상황을 대비해 배열로 관리)
    UPROPERTY()
    TArray<AAZPlayerCharacter*> LinkedTeamMembers;
    
    UFUNCTION()
    void OnEquipmentChanged(EEquipmentSlot SlotType, ESlotIndex SlotIndex, UAZItemBase* Item);

private:
    TWeakObjectPtr<AAZPlayerCharacter> CurrentTarget;
    void BindHealthDelegates(AAZPlayerCharacter* Target, bool bBind);
    
    UPROPERTY(EditAnywhere, Category = "AZ|Health")
    float TargetHealthPercent = 1.0f;  // 최종적으로 도달해야 할 체력 비율
    UPROPERTY(EditAnywhere, Category = "AZ|Health")
    float GhostDecreaseRate = 5.0f;    // 초당 감소 속도
    bool bIsGhostDecreasing = false;   // 감소 중인지 여부
    FTimerHandle GhostDelayTimerHandle;
    void StartGhostDecrease();
};
