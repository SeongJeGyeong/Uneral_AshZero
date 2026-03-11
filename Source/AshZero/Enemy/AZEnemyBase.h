// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Util/AZDefine.h"
#include "AZEnemyBase.generated.h"

class UAZHealthComponent;
class UAZEnemyFSM;
class UAIPerceptionComponent;
class UNavigationInvokerComponent;
class UAnimMontage;
class UAZEnemyHealthUIComponent;
class UAudioComponent;

UENUM(BlueprintType)
enum class EDeathFadeType : uint8
{
    Opacity     UMETA(DisplayName = "Opacity Fade"),
    Dissolve    UMETA(DisplayName = "Dissolve")
};

UCLASS()
class ASHZERO_API AAZEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAZEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AZ|Components")
    TObjectPtr<UAZHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AZ|Components")
    TObjectPtr<UAZEnemyFSM> FSMComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Components")
    TObjectPtr<UNavigationInvokerComponent> NavInvokerComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    UAZEnemyHealthUIComponent* HealthUIComponent;

public:
    UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "AZ|State")
    bool bIsDead = false;
protected:
    UFUNCTION()
    void OnRep_IsDead();

    // ===== 애니메이션 =====
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Animation")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Animation")
    TObjectPtr<UAnimMontage> HitReactMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Death")
    TObjectPtr<UAnimMontage> DeathMontage;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.0f);

	// ===== 데미지 및 사망 처리 =====
protected:
    UFUNCTION()
    void OnTakeAnyDamageCallback(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    virtual void OnDeath(FVector DeathLocation, AActor* Killer);


public:
	// ===== 사망 이펙트 설정 =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Death")
    float DeathLifeSpan = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    float DeathFadeDuration = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    float DeathFadeDelay = 5.0f;  // 죽음 애니메이션 후 페이드 시작까지 대기

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    TArray<UMaterialInterface*> DeathMaterials;  // Translucent 머티리얼

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    class UNiagaraSystem* DeathVFX;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    class USoundBase* DeathSound;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    FVector DeathVFXOffset = FVector(0.f, 0.f, 0.f);

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    FVector DeathVFXScale = FVector(1.f);

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death")
    EDeathFadeType DeathFadeType = EDeathFadeType::Dissolve;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death", meta = (EditCondition = "DeathFadeType == EDeathFadeType::Opacity"))
    FName OpacityParameterName = TEXT("Opacity");

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Death", meta = (EditCondition = "DeathFadeType == EDeathFadeType::Dissolve"))
    FName DissolveParameterName = TEXT("DissolveAmount");

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayDeathEffect(FVector Location);
    
	// ===== 주변음 설정 =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Audio")
    UAudioComponent* AmbientSoundComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Audio")
    USoundBase* AmbientLoopSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientSoundVolume = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Audio")
    USoundAttenuation* AmbientSoundAttenuation;

	// ===== 아이템 드랍 설정 =====
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Loot")
    TSubclassOf<class AActor> ItemDropClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Loot")
    float ItemDropChance = 0.3f;


	// ===== 유틸리티 함수 =====
    UFUNCTION(BlueprintPure, Category = "AZ|Health")
    UAZHealthComponent* GetHealthComponent() const { return HealthComponent; }

    UFUNCTION(BlueprintCallable, Category = "AZ|Movement")
    void SetMovementSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "AZ|Movement")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "AZ|Movement")
    float GetMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "AZ|Movement")
    float GetTurnRate() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "AZ|Effects")
    void OnPlayDeathEffect();

    UFUNCTION(BlueprintImplementableEvent, Category = "AZ|Effects")
    void OnPlayHitEffect(FVector HitLocation);

    UFUNCTION(BlueprintCallable, Category = "AZ|Target")
    virtual AActor* GetCurrentTarget() const;

public:
    UPROPERTY(ReplicatedUsing = OnRep_IsRoomDormant, BlueprintReadOnly, Category = "AZ|Room")
    bool bIsRoomDormant = false;

    void SetRoomDormant(bool bDormant);
protected:

    UFUNCTION()
    void OnRep_IsRoomDormant();

    void ApplyRoomDormantState(bool bDormant);

    // Dormant 진입 시 원복을 위한 캐시
    float CachedGravityScale = 1.0f;
    TEnumAsByte<EMovementMode> CachedMovementMode = MOVE_Walking;
    bool bCachedActorTickEnabled = true;
    bool bCachedMovementTickEnabled = true;
    bool bCachedFSMTickEnabled = true;
    bool bCachedNavInvokerTickEnabled = true;
    TEnumAsByte<ECollisionEnabled::Type> CachedCapsuleCollision = ECollisionEnabled::QueryAndPhysics;

    // 사망 페이드 관련
    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> DynamicDeathMaterials;

    FTimerHandle DeathFadeDelayHandle;
    FTimerHandle DeathFadeTimerHandle;
    float DeathFadeElapsed = 0.f;

    void StartDeathFade();
    void UpdateDeathFade();
    void FinishDeathFade();
    void DropLootBox();

private:
    float PreviousYaw = 0.f;
    float CurrentTurnRate = 0.f;
};
