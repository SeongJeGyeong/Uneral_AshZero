// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AZItemUsageComponent.h"
#include "System/Player/AZPlayerController.h"
#include "Character/AZPlayerCharacter.h"
#include "EnhancedInputComponent.h" 
#include "EnhancedInputSubsystems.h"
#include "AZDataAsset.h"
#include "AZGameplayTags.h"
#include "Engine/Engine.h"
#include "Item/AZItemBase.h"
#include "System/AZDataManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "DataTable/AZSuppliesItemDataTableRow.h"
#include "Components/AZHealthComponent.h"
#include "Components/AZEquipmentComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Animation/AZPlayerAnimInstance.h"
#include "DataTable/AZThrowablesItemDataTable.h"
#include "Components/AZPlayerFireComponent.h"

// Sets default values for this component's properties
UAZItemUsageComponent::UAZItemUsageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// ...
}

void UAZItemUsageComponent::SetupInputBinding(class UEnhancedInputComponent* PlayerInputComponent)
{
	if (PlayerInputComponent == nullptr) return;
	
	AAZPlayerController* OwningPC = Cast<AAZPlayerController>(GetOwner());
	if (OwningPC == nullptr)
		return;
	EquipmentComponent = OwningPC->GetComponentByClass<UAZEquipmentComponent>();
	if (EquipmentComponent == nullptr)
		return;

	AAZPlayerCharacter* OwnerCharacter = Cast<AAZPlayerCharacter>(OwningPC->GetPawn());
	if (OwnerCharacter == nullptr) return;

	UAZDataAsset* InputData = OwnerCharacter->InputDataAsset;
	if (InputData == nullptr) return;

	// 1. 매핑 컨텍스트 추가 (컨트롤러가 확실히 존재할 때 실행됨)

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningPC->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputData->IMC_TPS, 0);
	}


	// 2. 액션 바인딩 (인자로 받은 컴포넌트 사용)
	const UInputAction* QuickSlotNum1Action = InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Num1);
	if (QuickSlotNum1Action)
		PlayerInputComponent->BindAction(QuickSlotNum1Action, ETriggerEvent::Started, this, &UAZItemUsageComponent::OnQuickSlot_1);

	const UInputAction* QuickSlotNum2Action = InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Num2);
	if (QuickSlotNum2Action)
		PlayerInputComponent->BindAction(QuickSlotNum2Action, ETriggerEvent::Started, this, &UAZItemUsageComponent::OnQuickSlot_2);

	const UInputAction* QuickSlotNum3Action = InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Num3);
	if (QuickSlotNum3Action)
		PlayerInputComponent->BindAction(QuickSlotNum3Action, ETriggerEvent::Started, this, &UAZItemUsageComponent::OnQuickSlot_3);

	const UInputAction* QuickSlotNum4Action = InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Num4);
	if (QuickSlotNum4Action)
		PlayerInputComponent->BindAction(QuickSlotNum4Action, ETriggerEvent::Started, this, &UAZItemUsageComponent::OnQuickSlot_4);
}

void UAZItemUsageComponent::OnQuickSlot_1(const FInputActionValue& Value)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("num: 1"));
	if (EquipmentComponent == nullptr)
		return;
	if (EquipmentComponent->QuickSlotItem_1 == nullptr)
		return;
	ProcessItemUsage(EquipmentComponent->QuickSlotItem_1, ESlotIndex::Slot_1);
}

void UAZItemUsageComponent::OnQuickSlot_2(const FInputActionValue& Value)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("num: 2"));
	if (EquipmentComponent == nullptr)
		return;
	if (EquipmentComponent->QuickSlotItem_2 == nullptr)
		return;
	ProcessItemUsage(EquipmentComponent->QuickSlotItem_2, ESlotIndex::Slot_2);
}

void UAZItemUsageComponent::OnQuickSlot_3(const FInputActionValue& Value)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("num: 3"));
	if (EquipmentComponent == nullptr)
		return;
	if (EquipmentComponent->QuickSlotItem_3 == nullptr)
		return;
	ProcessItemUsage(EquipmentComponent->QuickSlotItem_3, ESlotIndex::Slot_3);
}

void UAZItemUsageComponent::OnQuickSlot_4(const FInputActionValue& Value)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("num: 4"));
	if (EquipmentComponent == nullptr)
		return;
	if (EquipmentComponent->QuickSlotItem_4 == nullptr)
		return;
	ProcessItemUsage(EquipmentComponent->QuickSlotItem_4, ESlotIndex::Slot_4);
}

bool UAZItemUsageComponent::IsUsingItem()
{
	if (GetWorld())
	{
		return GetWorld()->GetTimerManager().IsTimerActive(ItemUseTimerHandle);
	}
	return false;
}

void UAZItemUsageComponent::RequestUseItem_Server_Implementation(UAZItemBase* ItemToUse, ESlotIndex SlotIndex)
{
	ProcessItemUsage(ItemToUse, SlotIndex);
}

// Called when the game starts
void UAZItemUsageComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}


void UAZItemUsageComponent::ProcessItemUsage(UAZItemBase* ItemToUse, ESlotIndex SlotIndex)
{
	if (ItemToUse == nullptr) return;
	if (GetWorld()->GetTimerManager().IsTimerActive(ItemUseTimerHandle)) return;
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr) return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr) return;

	AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
	if (PC == nullptr) return;
	AAZPlayerCharacter* PlayerCharacter = Cast<AAZPlayerCharacter>(PC->GetPawn());
	if (PlayerCharacter == nullptr) return;
	UAZPlayerAnimInstance* AnimInst = Cast<UAZPlayerAnimInstance>(PlayerCharacter->GetMesh()->GetAnimInstance());
	if (AnimInst == nullptr) return;

	
	if (ItemToUse->GetItemType() == EItemType::Supplies)
	{
		const FAZSuppliesItemDataTableRow* SuppliesData = DataManger->GetItemDataByID<FAZSuppliesItemDataTableRow>(ItemToUse->GetItemID());
		if (SuppliesData == nullptr) return;

		if (SuppliesData->UseTime_Sec > 0.0f)
		{
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject(this, &UAZItemUsageComponent::ApplySuppliesEffect, ItemToUse, SuppliesData);

			GetWorld()->GetTimerManager().SetTimer(ItemUseTimerHandle, TimerDelegate, SuppliesData->UseTime_Sec, false);
			//EquipmentComponent->SubQuickSlotItem(SlotIndex);
			EquipmentComponent->SetUsedQuickSlotIndex(SlotIndex);
			PlayerCharacter->PlayItemUseAnim_Server();
			PlayerCharacter->PlaySFX_Server(ESFXType::Bandage);
		}
		else
		{
			ApplySuppliesEffect(ItemToUse, SuppliesData);
		}
	}
	else if (ItemToUse->GetItemType() == EItemType::Throwables)
	{
		const FAZThrowablesItemDataTable* ThrowablesData = DataManger->GetItemDataByID<FAZThrowablesItemDataTable>(ItemToUse->GetItemID());
		if (ThrowablesData == nullptr) return;

		float Duration = 1.53f;
		GetWorld()->GetTimerManager().SetTimer(ItemUseTimerHandle, Duration, false);

		if (PlayerCharacter->FireComp)
		{
			PlayerCharacter->FireComp->StartThrowGrenade_Server(ItemToUse->GetItemID());
			EquipmentComponent->SetUsedQuickSlotIndex(SlotIndex);
		}
	}
}

void UAZItemUsageComponent::ApplySuppliesEffect(UAZItemBase* Item, const FAZSuppliesItemDataTableRow* Data)
{
	if (EquipmentComponent == nullptr || Item == nullptr)
		return;
	AAZPlayerController* OwningPC = Cast<AAZPlayerController>(GetOwner());
	if (OwningPC == nullptr)
		return;
	AAZPlayerCharacter* PlayerCharacter = Cast<AAZPlayerCharacter>(OwningPC->GetCharacter());
	if (PlayerCharacter == nullptr)
		return;
	UAZHealthComponent* HealthComponent = PlayerCharacter->HealthComp;
	if (HealthComponent == nullptr)
		return;
	if (HealthComponent->Hp <= 0) return;


	if (Data->HealAmount_HP > 0.0f)
	{
		HealthComponent->AddHealth(Data->HealAmount_HP);
	}

	if (Data->Thirst_Restore > 0.0f)
	{
		HealthComponent->AddHydration(Data->Thirst_Restore);
	}

	if (Data->Hunger_Restore > 0.0f)
	{
		HealthComponent->AddFullness(Data->Hunger_Restore);
	}

	PlayerCharacter->PlayItemUseVFX_Server(HealingVFX);
	PlayerCharacter->PlaySFX_Server(ESFXType::Healing);
	
}

void UAZItemUsageComponent::ApplyThrowablesEffect(UAZItemBase* Item, const FAZThrowablesItemDataTable* Data)
{
}

// Called every frame
void UAZItemUsageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


