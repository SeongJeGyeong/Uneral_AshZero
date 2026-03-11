// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Util/AZDefine.h"
#include "AZDeveloperSettings.generated.h"

class AAZChest;
class UAZSoundDataAsset;
class UAZEnemyDataAsset;
class UAZSequenceDataAsset;
class UAZUISoundDataAsset;
class UAZBagShapeDataAsset;

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "AZ Data Settings"))
class ASHZERO_API UAZDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
    // 프로젝트 세팅창에서 할당할 클래스 참조
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TSubclassOf<AAZChest> RewardBoxClass;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TSoftObjectPtr<UDataTable> QuestDataTable;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Dialog")
    TSoftObjectPtr<UDataTable> DialogDataTable;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Item")
    TArray<TSoftObjectPtr<UDataTable>> ItemDataTableList;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> LoadingScreenClass;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tips")
    TSoftObjectPtr<UDataTable> TipsDataTable;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cinematics")
    TSubclassOf<UUserWidget> CutsceneWidgetClass;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cinematics")
    TSoftObjectPtr<UMediaPlayer> ScenePlayer;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cinematics")
    TMap<ECutsceneType, FCutscene> CutsceneMap;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cinematics")
    TSoftObjectPtr<UAZSequenceDataAsset> SequenceData;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Sound")
    TSoftObjectPtr<UAZSoundDataAsset> SoundMap;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Enemy")
    TSoftObjectPtr<UAZEnemyDataAsset> EnemyMap;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TSoftObjectPtr<UAZUISoundDataAsset> UISoundDataTable;

    //아이템 데이터 테이블
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|ItemData")
    TSoftObjectPtr<UDataTable> MiscDataTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|ItemData")
    TSoftObjectPtr<UDataTable> MaterialDataTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|ItemData")
    TSoftObjectPtr<UDataTable> SuppliesDataTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|ItemData")
    TSoftObjectPtr<UDataTable> ThrowablesDataTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|ItemData")
    TSoftObjectPtr<UDataTable> WeaponDataTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|ItemData")
    TSoftObjectPtr<UDataTable> EquipmentDataTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|ItemData")
    TSoftObjectPtr<UDataTable> StorageDataTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|ItemData")
    TSoftObjectPtr<UDataTable> QuestItemDataTable;

    //드랍 테이블
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|Drop")
    TSoftObjectPtr<UDataTable> BioDropTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|Drop")
    TSoftObjectPtr<UDataTable> MechDropTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|Drop")
    TSoftObjectPtr<UDataTable> LootBoxDropTable;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|Drop")
    TSoftObjectPtr<UDataTable> BossDropTable;


    //제작 테이블
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|Crafting")
    TSoftObjectPtr<UDataTable> CraftingRecipeDataTable;

    //상점 테이블
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|Drop")
    TSoftObjectPtr<UDataTable> StoreDataTable;

    // 가방 데이터 에셋
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AZ|Bag")
    TSoftObjectPtr<UAZBagShapeDataAsset> BagShapeDataAsset;
public:
    virtual FName GetCategoryName() const override;

#if WITH_EDITOR
    virtual FText GetSectionText() const override;
    virtual FText GetSectionDescription() const override;
#endif
};
