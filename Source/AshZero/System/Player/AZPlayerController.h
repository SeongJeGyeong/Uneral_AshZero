// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Util/AZDefine.h"
#include "AZPlayerController.generated.h"

/**
 * 
 */
class UAZInventoryWidget;
class UAZBagShapeDataAsset;
class UAZItemBase;
class UAZInventoryComponent;
class UAZPlayerInventoryComponent;
class UAZInteractionUIComponent;
class UAZInteractionWidget;
class UAZItemWidget;
class UAZDialogUIComponent;
class UAZSessionUIComponent;
class UInputAction;
class UAZLootBoxComponent;
class AAZChest;
class UAZBagItem;
class UAZItemUsageComponent;
class UAZCraftingWidget;
class UAZStashComponent;
class UAZEquipmentComponent;
class UAZDataManagerSubsystem;
class AAZWeapon;
class UAZStoreWidget;
class UAZExitUI;

UCLASS()
class ASHZERO_API AAZPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
protected:
	virtual void ClientRestart_Implementation(APawn* NewPawn) override;
	//virtual void SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;
	virtual void SetupInputComponent() override;

public:
	void SetHUDVisibility(ESlateVisibility Visibility);
	void SetMouseMode(bool bShow);
	void SetPartyUI();

public:
	AAZPlayerController();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	//인벤토리
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Component")
	TObjectPtr<UAZPlayerInventoryComponent> InventoryComp;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Component")
	TObjectPtr<UAZEquipmentComponent> EquipmentComp;
	//창고
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Component")
	TObjectPtr<UAZStashComponent> StashComp;

	UAZInventoryWidget* GetInventoryWidget();
	UPROPERTY(EditAnywhere, Category = "AZ|UI")
	TSubclassOf<UAZInventoryWidget> InventoryWidgetClass;

	UPROPERTY()
	TObjectPtr<UAZInventoryWidget> InventoryWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Component")
	TObjectPtr<UAZInteractionUIComponent> InteractionUIComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Component")
	TObjectPtr<UAZDialogUIComponent> DialogUIComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Component")
	TObjectPtr<UAZSessionUIComponent> SessionUIComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Component")
	TObjectPtr<UAZItemUsageComponent> ItemUsageComp;

	UPROPERTY(EditAnywhere, Category = "AZ|UI")
	TSubclassOf<UUserWidget> SpectateWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|UI")
	TSubclassOf<UAZExitUI> WarpWidgetClass;

	UPROPERTY()
	UAZExitUI* WarpWidget;

	UPROPERTY(ReplicatedUsing = OnRep_SpectateTarget)
	TObjectPtr<APawn> TargetPawn;

	float CameraBlendTime = 0.5f;
	float BlendStartTime = 0.f;

	//루트박스 열고 닫기
	UFUNCTION(Server, Reliable)
	void RequestOpenLootBox_Server(UAZLootBoxComponent* LootBoxComponent, AAZChest* Chest);

	UFUNCTION(Server, Reliable)
	void CloseLootBox_Server();


	UFUNCTION(Server, Reliable)
	void MoveItem_Server(UAZInventoryComponent* SourceComp, UAZInventoryComponent* DestComp, UAZItemBase* Item, int32 TopLeftIndex, bool bIsRotated);

	UFUNCTION(Server, Reliable)
	void RemoveItemToInventory_Server(UAZInventoryComponent* InventoryComponent, UAZItemBase* Item);

	UFUNCTION(Server, Reliable)
	void AddItemToInventory_Server(UAZInventoryComponent* InventoryComponent, UAZItemBase* Item, int32 TopLeftIndex, bool bIsRotated);

	UFUNCTION(Server, Reliable)
	void TryAddItemToInventory_Server(UAZInventoryComponent* InventoryComponent, UAZItemBase* Item);
	
	UFUNCTION(Server, Reliable)
	void SetItemDrag_Server(UAZItemBase* Item, bool bIsDrag);

	UFUNCTION(Client, Reliable)
	void ReturnToMenu_Client();

	UFUNCTION(Server, Reliable)
	void RefreshInventory_Server(UAZInventoryComponent* InventoryComponent);

	UFUNCTION(NetMulticast, Reliable)
	void RefreshInventory_Multicast(UAZInventoryComponent* InventoryComponent);

	void BindUIVisibilityRule();

	void ChangeCameraViewTarget(AActor* newTarget);

	UFUNCTION(Server, Reliable)
	void TryStackItem_Server(UAZInventoryComponent* SourceComponent, UAZInventoryComponent* DestComponent, UAZItemBase* SourceItem, UAZItemBase* DestItem, UAZItemWidget* SourceWidget, UAZItemWidget* DestWidget);

	//제작 UI
	UPROPERTY(EditAnywhere, Category = "AZ|UI")
	TSubclassOf<UAZCraftingWidget> CraftingWidgetClass;

	UPROPERTY()
	TObjectPtr<UAZCraftingWidget> CraftingWidget;

	UFUNCTION()
	void OnGameStateChanged(EGameState NewState);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "AZ|Networking")
	void PreTravelCleanUp_Client();
	void ClearAllWidgets();

	void HandleDeathFinished();

	UFUNCTION(Client, Reliable)
	void SetSpectatorView_Client(APawn* Target);

	void ReturnToLobby();

	UFUNCTION()
	void OnRep_SpectateTarget();

	void EnterRoadOut();

	void StartMapGenerate();
	void OnMapGenerationFinished();

	UFUNCTION(Server, Reliable)
	void NotifyMapLoadComplete_Server();

	// 서버가 모든 준비가 끝났을 때 호출하는 RPC
	UFUNCTION(NetMulticast, Reliable)
	void HideLoadingScreen_Multicast();

	void SwitchPartyUI();

	UFUNCTION(Server, Reliable)
	void InitializeStartingLoadout_Server();

	void SpawnWeaponActor(UAZItemBase* Item, ESlotIndex SlotIndex);

	UFUNCTION(Server, Reliable)
	void SetReady_Server(bool bReady);

	void ApplySaveData(const FAZPlayerSaveData& SaveData);
	UFUNCTION(Server, Reliable)
	void SyncPlayerData_Server(const FAZPlayerSaveData& SaveData);

	void CreateWarpWidget();
	void RemoveWarpWidget();
	void ShowWarpUI(bool bShow);
	void UpdateWarpProgress(float Progress);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_IsSaveLoaded)
	bool bIsSaveLoaded = false;

	UFUNCTION()
	void OnRep_IsSaveLoaded();

	UPROPERTY(Transient)
	TObjectPtr<AAZChest> CurrentOpenChest;
private:
	UAZItemBase* CreateItem(const FAZItemSaveData& ItemData, UAZDataManagerSubsystem* DataManager);

	UPROPERTY(EditAnywhere, Category = "AZ|UI")
	TSubclassOf<UAZStoreWidget> StoreWidgetClass;
};
