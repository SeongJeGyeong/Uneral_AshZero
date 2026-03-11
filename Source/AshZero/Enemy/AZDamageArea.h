// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AZStatusTypes.h"
#include "AZDamageArea.generated.h"

UCLASS()
class ASHZERO_API AAZDamageArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZDamageArea();

    UPROPERTY(VisibleAnywhere, Category = "AZ|Components")
    class USphereComponent* CollisionComp;

    // 나이아가라 이펙트를 BP에서 교체하여 독, 불, 얼음 등을 표현
    UPROPERTY(VisibleAnywhere, Category = "AZ|Visual")
    class UNiagaraComponent* AreaEffect;

    // 0이면 무한 유지, 그 외에는 초 단위 유지시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Config")
    float Duration = 5.0f;


    // 플레이어에게 전달할 상태이상 정보 묶음
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Config")
    FAZStatusEffectPacket EffectPacket;

    // 장판 안에 있는 동안 지속적으로 효과 적용할지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Config")
    bool bApplyWhileInside = true;

    // 장판 안에서 효과 적용 간격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Config", meta = (EditCondition = "bApplyWhileInside"))
    float ReapplyInterval = 1.0f;


protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    // 장판 안에 있는 플레이어들 추적
    UPROPERTY()
    TSet<class AAZPlayerCharacter*> PlayersInArea;

    // 지속 효과 적용 타이머
    FTimerHandle ReapplyTimerHandle;


    void ApplyEffectToPlayersInArea();
};
