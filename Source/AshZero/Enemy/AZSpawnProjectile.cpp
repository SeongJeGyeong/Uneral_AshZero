// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AZSpawnProjectile.h"
#include "Enemy/AZProjectile.h"
#include "Enemy/AZEnemyBase.h"
#include "AIController.h"
#include "Components/SphereComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

void UAZSpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !ProjectileClass) return;

	AAZEnemyBase* Enemy = Cast<AAZEnemyBase>(MeshComp->GetOwner());
	if (!Enemy) return;

	// 서버에서만 스폰
	if (!Enemy->HasAuthority()) return;

	// 스폰 위치
	FVector SpawnLocation = MeshComp->GetSocketLocation(SpawnSocket);
	FRotator SpawnRotation = Enemy->GetActorRotation();
	FVector LaunchDirection = SpawnRotation.Vector();

	// 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Enemy;
	SpawnParams.Instigator = Enemy;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAZProjectile* Projectile = MeshComp->GetWorld()->SpawnActor<AAZProjectile>(ProjectileClass,SpawnLocation,SpawnRotation,SpawnParams);

	if (!Projectile) return;


	//Projectile->CollisionComp->IgnoreActorWhenMoving(Enemy, true);

	//Enemy->MoveIgnoreActorAdd(Projectile);

	// ===== 발사 (소켓 정방향) =====
	Projectile->Launch(SpawnRotation.Vector());

}
