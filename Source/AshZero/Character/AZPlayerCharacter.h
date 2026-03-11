// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "AZStatusTypes.h"
#include "Interface/AZStatusInterface.h"
#include "Util/AZDefine.h"
#include "AZPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UEnhancedInputComponent;
class UAZPlayerMoveComponent;
class UAZDataAsset;
class UAZHealthComponent;
class UAZPlayerInventoryComponent;
class UAZInteractionComponent;
class UAZLootBoxComponent;
class UAZStatusBarWidget;
class UNiagaraSystem;
class USoundBase;


// Delegate 선언
DECLARE_MULTICAST_DELEGATE_OneParam(FInputBindingDelegate, class UEnhancedInputComponent*);

UCLASS()
class ASHZERO_API AAZPlayerCharacter : public ACharacter, public IGenericTeamAgentInterface, public IAZStatusInterface
{
    GENERATED_BODY()

public:
    AAZPlayerCharacter();

    virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(0); }

    // ===== IAZStatusInterface 구현 =====
    virtual void ApplyStatusEffect(const FAZStatusEffectPacket& Packet, AActor* Causer = nullptr) override;

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PawnClientRestart() override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CamComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAZPlayerMoveComponent* MoveComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAZHealthComponent* HealthComp;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Component")
    class UAZPlayerFireComponent* FireComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Component")
    UAZInteractionComponent* InteractionComp;

    // DataAsset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AZ|Input")
    UAZDataAsset* InputDataAsset;
    // Delegate
    FInputBindingDelegate OnInputBindingDelegate;

    // 무기
    UPROPERTY()
    class AAZWeapon* CurrentWeapon;

    // UI
    UPROPERTY(EditAnywhere, Category = "AZ|UI")
    TSubclassOf<UAZStatusBarWidget> StatusBarWidgetClass;

    UPROPERTY()
    UAZStatusBarWidget* StatusBarWidget;

    // === 사망 연출 ===
    UFUNCTION()
    void OnDeath(FVector DeathLocation, AActor* Killer);

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    float DeathAnimDuration = 3.2f;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    float FadeOutDuration = 3.3f;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    float ReturnToLobbyDelay = 6.0f;

    // 사망 UI 클래스
    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    TSubclassOf<UUserWidget> DeathWidgetClass;

    // 사망 VFX
    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    UNiagaraSystem* DeathVFX;

    // 사망 시 사용할 Translucent 머티리얼 (BP에서 설정)
    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    TArray<UMaterialInterface*> DeathMaterials;

    // 행동 가능 여부 체크
    UFUNCTION(BlueprintCallable, Category = "AZ|Action")
    bool CanPerformAction() const;

    // DoT 효과 직접 해제 (HealthComponent에서 호출용)
    UFUNCTION(BlueprintCallable, Category = "AZ|Status")
    void StopDoTEffect(EAZStatusType StatusType);


    // 사망 플래그 (다른 곳에서 접근 가능하도록)
    UPROPERTY(BlueprintReadOnly, Category = "AZ|Status")
    bool bIsDead = false;

    // === 피격 ===
    UFUNCTION()
    void HandleOnHit(float Damage, AActor* Causer, FVector HitLocation);

    FTimerHandle HitStunTimerHandle;

    // 피격 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "AZ|Animation")
    UAnimMontage* LightHitMontage;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Animation")
    UAnimMontage* HeavyHitMontage;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Animation")
    UAnimMontage* KnockbackMontage;

    UPROPERTY(BlueprintReadOnly, Category = "AZ|State")
    bool bIsHitStunned = false;

    // 피격 반응 처리
    UFUNCTION(Server, Reliable)
    void Server_PlayHitReaction(EHitReactionType ReactionType, FVector HitDirection, float KnockbackForce);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayHitReaction(EHitReactionType ReactionType, FVector HitDirection, float KnockbackForce);

    void ExecuteHitReaction(EHitReactionType ReactionType, FVector HitDirection, float KnockbackForce);

    // 모든 클라이언트에서 이펙트 재생
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayHitEffect();

    UPROPERTY(EditAnywhere, Category = "AZ|Effects")
    class UNiagaraSystem* BloodVFX;

    UPROPERTY(EditAnywhere, Category = "AZ|Effects")
    FName BloodSocketName = TEXT("BloodSocket"); 

    // 카메라 흔들림 클래스 (BP에서 할당)
    UPROPERTY(EditAnywhere, Category = "AZ|Effects")
    TSubclassOf<class UCameraShakeBase> HitCameraShakeClass;

    // 대미지 오버레이 위젯 클래스 (BP에서 할당)
    UPROPERTY(EditAnywhere, Category = "AZ|Effects")
    TSubclassOf<class UUserWidget> DamageOverlayClass;

    UPROPERTY()
    class UUserWidget* DamageOverlayWidget;

    UFUNCTION(Server, Unreliable)
    void PlayItemUseAnim_Server();

    UFUNCTION(NetMulticast, Unreliable)
    void PlayItemUseAnim_Multicast();

    UFUNCTION(Server, Unreliable)
    void PlayItemUseVFX_Server(UNiagaraSystem* VFX);

    UFUNCTION(NetMulticast, Unreliable)
    void PlayItemUseVFX_Multicast(UNiagaraSystem* VFX);

    UFUNCTION(Server, Unreliable)
    void PlaySFX_Server(ESFXType SFXType);

    UFUNCTION(NetMulticast, Unreliable)
    void PlaySFX_Multicast(ESFXType SFXType);

    // 피격 연출
    void PlayLocalHitEffects();

    // Post Process 블러 효과
    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
    class UMaterialInterface* VignetteMaterial;

    UPROPERTY()
    class UMaterialInstanceDynamic* VignetteMID;

    UPROPERTY(VisibleAnywhere, Category = "AZ|Effects")
    class UPostProcessComponent* VignettePostProcess;

    // 줌 시 비네트 강도
    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
    float ZoomVignetteIntensity = 20.0f;

    UFUNCTION()
    void OnGameStateChanged(EGameState NewState);
protected:
    // 사망 처리 함수들
    void StartDeathSequence();
    void StartDeathFade();
    void UpdateDeathFade();
    void FinishDeathSequence();
    //void ReturnToLobby();

    FTimerHandle DeathFadeStartHandle;
    FTimerHandle DeathFadeTimerHandle;
    FTimerHandle LobbyTimerHandle;
    float DeathFadeElapsed = 0.f;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

    UPROPERTY()
    UUserWidget* DeathWidget;

    UPROPERTY(EditAnywhere, Category = "AZ|Hair")
    UMaterialInstance* RemoteHairMaterial;


    UPROPERTY(EditAnywhere, Category = "AZ|Hair")
    UMaterialInstance* LocalHairMaterial;
 private:
    // 현재 걸려있는 상태 이상들의 타이머를 관리 (중복 적용 방지용)
    UPROPERTY()
    TMap<EAZStatusType, FTimerHandle> StatusTimerMap;

    // DoT(Damage over Time) 효과들 관리
    UPROPERTY()
    TMap<EAZStatusType, FActiveDoTEffect> ActiveDoTEffects;

    // 실제 효과 적용/제거 내부 함수
    void ExecuteStatusEffect(FAZStatusEffectPacket EffectPacket);
    void RemoveStatusEffect(EAZStatusType StatusType);

    // DoT 효과 관련 함수
    void StartDoTEffect(const FAZStatusEffectPacket& EffectPacket);
    void ProcessDoTTick(EAZStatusType StatusType);

    // DoT 타입인지 확인
    bool IsDoTEffect(EAZStatusType StatusType) const;

    void NotifyDeathFinished();

    void SetLocalPlayerMaterialParam();
};