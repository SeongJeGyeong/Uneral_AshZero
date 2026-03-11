// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Components/CanvasPanelSlot.h"
#include "MediaPlayer.h"
#include "AZDefine.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EHitReactionType : uint8
{
	None        UMETA(DisplayName = "None"),
	LightHit    UMETA(DisplayName = "Light Hit"),    
	HeavyHit    UMETA(DisplayName = "Heavy Hit"),    
	Knockback   UMETA(DisplayName = "Knockback")     
};

UENUM(BlueprintType)
enum class ETurnInPlace : uint8
{
	ETIP_Left UMETA(DisplayName = "Turn Left"),
	ETIP_Right UMETA(DisplayName = "Turn Right"),
	ETIP_NotTurning UMETA(DisplayName = "Not Turning"),
};

UENUM()
enum class EHighlightColor : uint32
{
	Red = 0xFF0000FF,
	Green = 0x00FF00FF,
	Yellow = 0x00FFFF00,
	White = 0xFFFFFFFF,
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Misc			UMETA(DisplayName = "Misc"),
	Material		UMETA(DisplayName = "Material"),
	Supplies		UMETA(DisplayName = "Supplies"),
	Throwables		UMETA(DisplayName = "Throwables"),
	Weapon_Firearm	UMETA(DisplayName = "Weapon_Firearm"),
	Equipment		UMETA(DisplayName = "Equipment"),
	Storage			UMETA(DisplayName = "Storage"),
	Quest			UMETA(DisplayName = "Quest"),
	None			UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None		UMETA(DisplayName = "None"),
	Helmet		UMETA(DisplayName = "Helmet"),
	Armor		UMETA(DisplayName = "Armor"),
	Gloves		UMETA(DisplayName = "Gloves"),
	Boots		UMETA(DisplayName = "Boots"),
	Weapon		UMETA(DisplayName = "Weapon"),
	Backpack	UMETA(DisplayName = "Backpack"),
	QuickSlot	UMETA(DisplayName = "QuickSlot"),
};

UENUM(BlueprintType)
enum class ESlotIndex : uint8
{
	Slot_0 = 0,
	Slot_1 = 1,
	Slot_2 = 2,
	Slot_3 = 3,
	Slot_4 = 4
};

UENUM(BlueprintType)
enum class EItemSourceType : uint8	//어느 아이템 창에서 왔는지 ex) 인벤토리, 장비창, 무기창
{
	Inventory		UMETA(DisplayName = "Inventory"),
	Equipment		UMETA(DisplayName = "Equipment"),
	FirstWeapon		UMETA(DisplayName = "FirstWeapon"),
	SecondWeapon	UMETA(DisplayName = "SecondWeapon"),
	QuickSlot_1		UMETA(DisplayName = "QuickSlot_1"),
	QuickSlot_2		UMETA(DisplayName = "QuickSlot_2"),
	QuickSlot_3		UMETA(DisplayName = "QuickSlot_3"),
	QuickSlot_4		UMETA(DisplayName = "QuickSlot_4"),
};

UENUM(BlueprintType)
enum class EItemBuffType : uint8
{
	None			UMETA(DisplayName = "None"),
	Invincibility	UMETA(DisplayName = "Invincibility"),
	DamageBoost		UMETA(DisplayName = "DamageBoost"),
};

UENUM(BlueprintType)
enum class EExplosionType: uint8
{
	Burst	UMETA(DisplayName = "Burst"),
	Smoke	UMETA(DisplayName = "Smoke"),
	Flash	UMETA(DisplayName = "Flash"),
};

UENUM(BlueprintType)
enum class ERarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
};

UENUM(BlueprintType)
enum class ELootSourceType : uint8
{
	None,
	BioMonster,
	MechMonster,
	LootBox,
	Shop,
	Crafting,
	Boss,
	DefaultItem,
};

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Lobby,
	InGame
};

UENUM(BlueprintType)
enum class EQuestObjectiveType : uint8
{
	Slay			UMETA(DisplayName = "토벌"),
	Collect			UMETA(DisplayName = "수집"),
	ReachLocation	UMETA(DisplayName = "도달"),
	Talk			UMETA(DisplayName = "대화"),
};

UENUM(BlueprintType)
enum class EQuestProgress : uint8
{
	BeforeStart,
	InProgress,
	Complete,
};

UENUM(BlueprintType)
enum class ESpeakerType : uint8
{
	Player,
	NPC,
	Custom,
};

UENUM(BlueprintType)
enum class EDialogType : uint8
{
	NormalTalk UMETA(DisplayName = "일반 대화문"),
	BeforeAcceptQuest UMETA(DisplayName = "퀘스트 수주전"),
	InProgressQuest UMETA(DisplayName = "퀘스트 진행중"),
	CompletedQuest UMETA(DisplayName = "퀘스트 완료"),
	AcceptQuest UMETA(DisplayName = "퀘스트 수락"),
	DeclineQuest UMETA(DisplayName = "퀘스트 거절"),
	Greetings UMETA(DisplayName = "인삿말")
};

UENUM(BlueprintType)
enum class EDialogButtonType : uint8
{
	Talk UMETA(DisplayName = "대화"),
	Quest UMETA(DisplayName = "퀘스트"),
	QuestAccept UMETA(DisplayName = "퀘스트 수락"),
	QuestDecline UMETA(DisplayName = "퀘스트 거절"),
	OpenShop UMETA(DisplayName = "상점창 열기"),
	OpenCrafting UMETA(DisplayName = "제작창 열기"),
	OpenEnhancement UMETA(DisplayName = "강화창 열기"),
	OpenSession UMETA(DisplayName = "세션창 열기"),
};

UENUM(BlueprintType)
enum class EBossType : uint8
{
	Argus,
	SpineQueen,
	Zephyros,
};

UENUM(BlueprintType)
enum class ECutsceneType : uint8
{
	Argus_Encounter,
	SpineQueen_Encounter,
	Zephyros_Encounter,
	Player_Enter,
};

UENUM(BlueprintType)
enum class EPartyIconState : uint8
{
	Empty,
	NoneReady,
	Ready,
};

UENUM()
enum class ERoomType : uint8
{
	Normal,
	Start,
	Boss,
	Teleport,
};

UENUM()
enum class EBGMType : uint8
{
	BGM_Title,
	BGM_Hub,
	BGM_Party,
	BGM_Field,
	BGM_Boss,
	BGM_Boss_Enter,
	BGM_Death,
};

//UENUM()
//enum class EActorType : uint8
//{
//	A_Player,
//	A_Frog,
//	A_Drone,
//	A_Robot,
//	A_Argus,
//	A_SpineQueen,
//	A_Zephyros,
//};

UENUM()
enum class EActorType : uint8
{
	A_Player,
	A_Enemy,
	A_Boss,
	A_Door,
};

UENUM()
enum class ESFXType : uint8
{
	None,
	FootStep,
	Bandage,
	Healing,
	Grenade_Pin,
	Greande_Boom,
	Door_Open,
	Hitted,
};

UENUM()
enum class EUISFXType : uint8
{
	None,
	Click,
	Hover,
	Transaction,
	Craft,
	Equip,
};


UENUM(BlueprintType)
enum class ESurfaceType : uint8
{
	Concrete,
	Metal,
};


UENUM(BlueprintType)
enum class EAZTradeMode : uint8
{
	Buy     UMETA(DisplayName = "Buy"),
	Sell    UMETA(DisplayName = "Sell")
};

USTRUCT(BlueprintType)
struct FWidgetLayoutData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAnchors Anchors; // 앵커 (예: 우하단 고정, 중앙 등)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMargin Offsets; // 좌표 및 크기 (Position X, Y, Size X, Y)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Alignment; // 정렬 기준점 (0.5, 0.5 등)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position; // 정렬 기준점 (0.5, 0.5 등)
};

USTRUCT(BlueprintType)
struct FBagDefinition
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag Shape", meta = (ClampMin = "0"))
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BagName = "New Bag";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag Shape", meta = (ClampMin = "1"))
	int32 Rows = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag Shape", meta = (ClampMin = "1"))
	int32 Columns = 6;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bag Shape", meta = (HideInDetailsView))
	TArray<bool> GridFlags;

	FBagDefinition()
	{
		GridFlags.Init(false, Rows * Columns);
	}


	bool operator==(const FBagDefinition& Other) const
	{
		return BagName == Other.BagName &&
			Rows == Other.Rows &&
			Columns == Other.Columns &&
			GridFlags == Other.GridFlags;
	}

	bool IsValid() const
	{
		return ID > 0;
	}
};

USTRUCT()
struct FLines
{
	GENERATED_USTRUCT_BODY();

	FLines()
	{

	};

	TArray<FVector2D> XLines;
	TArray<FVector2D> YLines;
};


USTRUCT()
struct FMousePositionInTile
{
	GENERATED_USTRUCT_BODY();

	FMousePositionInTile()
	{

	};

	bool bRight;
	bool bDown;
};

USTRUCT(BlueprintType)
struct FAZDialogChoice
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, meta = (DisplayName = "선택지 텍스트"))
	FText ButtonText;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "선택지 타입"))
	EDialogButtonType ButtonType;
};

USTRUCT(BlueprintType)
struct FAZDialogRow
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, meta = (DisplayName = "화자"))
	ESpeakerType Speaker;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "Speaker == ESpeakerType::Custom", DisplayName = "화자 선택"))
	FGameplayTag CustomSpeakerTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, meta = (MultiLine, DisplayName = "대사"))
	FText Line;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "선택지 있음"))
	bool bHasChoices;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bHasChoices", DisplayName = "선택지 목록"))
	TArray<FAZDialogChoice> Choices;
};

USTRUCT(BlueprintType)
struct FAZDialog : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	EDialogType DialogType;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ConversationNPC;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ContextTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FAZDialogRow> DialogRows;
};

USTRUCT(BlueprintType)
struct FAZQuestObjectiveData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, meta = (DisplayName = "목표 타입"))
	EQuestObjectiveType Type;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "목표 태그"))
	FGameplayTag ObjectiveTag;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "요구 횟수"))
	int32 RequiredCount = 1;

	UPROPERTY(Transient)
	int32 CurrentCount = 0;

	UPROPERTY(Transient)
	bool bCompleted = false;
};

USTRUCT()
struct FQuestObjectiveProgress
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	int32 CurrentCount = 0;

	UPROPERTY()
	bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct FAZQuestReward
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, meta = (DisplayName = "아이템 태그"))
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "개수"))
	int32 Amount;
};

USTRUCT(BlueprintType)
struct FAZQuest : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName QuestName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Categories = "Quest.ID"))
	FGameplayTag QuestTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Categories = "NPC"))
	FGameplayTag QuestGiverTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (MultiLine))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Categories = "Quest.ID"))
	FGameplayTagContainer PrerequisiteQuests;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FAZQuestObjectiveData> Objectives;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FAZQuestReward> Rewards;

	bool bIsComplete = false;
};

USTRUCT(BlueprintType)
struct FEnableButtons
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "DialogueType")
	bool bCrafting = false;

	UPROPERTY(EditDefaultsOnly, Category = "DialogueType")
	bool bTrade = false;

	UPROPERTY(EditDefaultsOnly, Category = "DialogueType")
	bool bEnhance = false;

	UPROPERTY(EditDefaultsOnly, Category = "DialogueType")
	bool bSession = false;
};

USTRUCT(BlueprintType)
struct FCutscene
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMediaSource> MediaSource;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInterface> MediaMaterial;

	UPROPERTY(EditDefaultsOnly)
	FText SceneText;
};

USTRUCT(BlueprintType)
struct FItemAmountMap
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TMap<int32, int32> Map;
};


USTRUCT(BlueprintType)
struct FAZItemSaveData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	int32 ItemId;

	UPROPERTY(BlueprintReadWrite)
	int32 Count;

	UPROPERTY(BlueprintReadWrite)
	FIntPoint Index;

	UPROPERTY(BlueprintReadWrite)
	bool bIsBagItem = false;

	UPROPERTY(BlueprintReadWrite)
	FBagDefinition BagStruct;
};

USTRUCT(BlueprintType)
struct FAZEquipmentSavePair
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EEquipmentSlot Slot = EEquipmentSlot::None;

	UPROPERTY(BlueprintReadWrite)
	FAZItemSaveData ItemData;
};

USTRUCT(BlueprintType)
struct FAZQuickSlotSavePair
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	FAZItemSaveData ItemData;
};

USTRUCT(BlueprintType)
struct FAZPlayerSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<FAZItemSaveData> InventoryItems;

	UPROPERTY(BlueprintReadWrite)
	TArray<FAZItemSaveData> StashItems;

	UPROPERTY(BlueprintReadWrite)
	TArray<FAZEquipmentSavePair> EquipmentItems;

	UPROPERTY(BlueprintReadWrite)
	TArray<FAZQuickSlotSavePair> QuickSlotItems;

	UPROPERTY(BlueprintReadWrite)
	FBagDefinition PlayerIventoryStruct;

	UPROPERTY(BlueprintReadWrite)
	FBagDefinition StashInventoryStruct;
};