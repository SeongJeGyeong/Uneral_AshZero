// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/AZDefine.h"
#include "AZInventoryWidget.generated.h"

/**
 * 
 */

class UAZInventoryComponent;
class UCanvasPanel;
class UBorder;
class UBackgroundBlur;
class UAZInventoryGridWidget;
class UAZBagShapeDataAsset;
class UAZBagSlotWidget;
class UAZEquipmentSlot;
class UOverlay;
class UHorizontalBox;
class UVerticalBox;
class UProgressBar;
class UAZHealthComponent;
class UAZEquipmentComponent;
class UButton;
class UTextBlock;


UCLASS()
class ASHZERO_API UAZInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	TObjectPtr<UAZInventoryComponent> PlayerInventoryComponent;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
public:
	UPROPERTY(EditAnywhere, Category = "UI Layout")
	TMap<EGameState, FWidgetLayoutData> InventoryLayouts;

	UPROPERTY(EditAnywhere, Category = "UI Layout")
	TMap<EGameState, FWidgetLayoutData> FirstGunSlotLayouts;

	UPROPERTY(EditAnywhere, Category = "UI Layout")
	TMap<EGameState, FWidgetLayoutData> SecondGunSlotLayouts;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UCanvasPanel> Canvas;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	UOverlay* EquipmnetOverlay;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UBorder> BackgroundBorder;

	//인벤토리 박스
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UVerticalBox> InventoryVerticalBox;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZInventoryGridWidget> PlayerInventoryGrid;

	// 선택한 인벤토리 박스
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UVerticalBox> SelectedInventoryVerticalBox;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZInventoryGridWidget> BagInventoryGrid;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZBagSlotWidget> SelectBagSlot;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZInventoryGridWidget> LootInventoryGrid;

	//창고 UI
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZInventoryGridWidget> StashInventoryGrid;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UVerticalBox> StashVerticalBox;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZBagSlotWidget> PlayerBagSlot;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> HelmetSlot;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> ArmorSlot;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> GlovesSlot;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> BootsSlot;

	//무기
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> FirstWeaponSlot;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> SecondWeaponSlot;

	//무기 VerticalBox
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UVerticalBox> FirstWeaponVerticalBox;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UVerticalBox> SecondWeaponVerticalBox;
	
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> QuickSlot_1;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> QuickSlot_2;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> QuickSlot_3;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UAZEquipmentSlot> QuickSlot_4;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UProgressBar> ThirstyProgressBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UProgressBar> HungerProgressBar;


	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UBackgroundBlur> BackgroundBlur;

	UPROPERTY(EditAnywhere, Category = "AZ|Inventory")
	TObjectPtr<UAZBagShapeDataAsset> BagShapeDataAsset;

	UFUNCTION()
	void SetUIMode(EGameState NewMode);
	void ApplyLayoutToWidget(UCanvasPanelSlot* CanvasSlot, const FWidgetLayoutData& LayoutData);

	void InitInventoryWidget(UAZEquipmentComponent* EquipmentComponent);
private:

	UFUNCTION()
	void UpdateSurvivalStats(float CurrentFullness, float MaxFullness, float CurrentHydration, float MaxHydration);

	UFUNCTION()
	void OnEquipmentChanged(EEquipmentSlot SlotType, ESlotIndex SlotIndex, UAZItemBase* Item);

	UFUNCTION()
	void OnCloseButtonClicked();
};
