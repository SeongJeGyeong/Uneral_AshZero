// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AZDamageArea.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Character/AZPlayerCharacter.h" // 플레이어 캐스팅용

// Sets default values
AAZDamageArea::AAZDamageArea()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = CollisionComp;

    AreaEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AreaEffect"));
    AreaEffect->SetupAttachment(RootComponent);

    // 이벤트 바인딩
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AAZDamageArea::OnOverlapBegin);
    CollisionComp->OnComponentEndOverlap.AddDynamic(this, &AAZDamageArea::OnOverlapEnd);
}

// Called when the game starts or when spawned
void AAZDamageArea::BeginPlay()
{
	Super::BeginPlay();
	
    if (Duration > 0.0f)
    {
        SetLifeSpan(Duration);
    }
    // 지속 효과 타이머 시작 (서버에서만)
    if (bApplyWhileInside && HasAuthority())
    {
        GetWorldTimerManager().SetTimer(
            ReapplyTimerHandle,
            this,
            &AAZDamageArea::ApplyEffectToPlayersInArea,
            ReapplyInterval,
            true  // 반복
        );
    }

}

void AAZDamageArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(OtherActor);
    if (!Player || !Player->IsPlayerControlled()) return;

    // 서버에서만 처리
    if (!HasAuthority()) return;

  
    // 플레이어 추적 목록에 추가
    PlayersInArea.Add(Player);

    // 즉시 효과 적용 (첫 진입 시)
    Player->ApplyStatusEffect(EffectPacket);
}

void AAZDamageArea::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(OtherActor);
    if (!Player) return;

    // 플레이어 추적 목록에서 제거
    PlayersInArea.Remove(Player);
}

void AAZDamageArea::ApplyEffectToPlayersInArea()
{

    // 유효하지 않은 플레이어 제거 (사망 또는 파괴된 경우)
    TArray<AAZPlayerCharacter*> InvalidPlayers;

    for (AAZPlayerCharacter* Player : PlayersInArea)
    {
        if (!IsValid(Player))
        {
            InvalidPlayers.Add(Player);
            continue;
        }

        // 상태이상 재적용 (Duration 갱신됨)
        Player->ApplyStatusEffect(EffectPacket);
    }

    // 유효하지 않은 플레이어 제거
    for (AAZPlayerCharacter* Invalid : InvalidPlayers)
    {
        PlayersInArea.Remove(Invalid);
    }
}