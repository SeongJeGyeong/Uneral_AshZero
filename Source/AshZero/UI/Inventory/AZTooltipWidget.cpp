// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/AZTooltipWidget.h"
#include "Item/AZItemBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/CanvasPanel.h"
#include "System/AZDataManagerSubsystem.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Util/AZDefine.h"

void UAZTooltipWidget::NativeConstruct()
{
	Super::NativeConstruct();

    if (ToolTipOverlay)
    {
        UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ToolTipOverlay->Slot);
        if (CanvasSlot)
        {
            CanvasSlot->SetSize(FVector2D(Width, Height));
        }
    }

}

void UAZTooltipWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (!IsVisible()) return;
    
    APlayerController* PC = GetOwningPlayer();
    if (PC == nullptr) return;

    float MouseX, MouseY;
    if (PC->GetMousePosition(MouseX, MouseY))
    {
        // 툴팁 기본 위치 (마우스 오른쪽 아래)
        FVector2D TooltipPos(MouseX + 15.0f, MouseY + 15.0f);

        FVector2D ViewportSize;
        GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);

        // [핵심 수정] 뷰포트 스케일(DPI)을 가져와서 위젯 크기에 곱해줌
        float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
        FVector2D WidgetPixelSize = GetDesiredSize() * ViewportScale;

        // 화면 오른쪽을 벗어나는지 체크 (픽셀 단위끼리 비교)
        if (TooltipPos.X + WidgetPixelSize.X > ViewportSize.X)
        {
            // 마우스 왼쪽으로 툴팁 이동 (위젯의 실제 픽셀 너비만큼 뺌)
            TooltipPos.X = MouseX - WidgetPixelSize.X - 5.0f;
        }

        // 화면 아래쪽을 벗어나는지 체크
        if (TooltipPos.Y + WidgetPixelSize.Y > ViewportSize.Y)
        {
            TooltipPos.Y = MouseY - WidgetPixelSize.Y - 5.0f;
        }

        SetPositionInViewport(TooltipPos);
    }
}

bool UAZTooltipWidget::UpdateToolTipInfo(UAZItemBase* Item)
{
    if (Item == nullptr) return false;
    
    if (!ItemImage || !ItemNameText || !DescriptionText) return false;

    UWorld* World = GetWorld();
    if (World == nullptr) return false;

    UGameInstance* GI = World->GetGameInstance();
    if (GI == nullptr) return false;

    UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>();
    if (DataManager == nullptr) return false;

    const FAZBaseItemDataTable* ItemDataRow = DataManager->GetItemDataByID(Item->GetItemID());
    if (ItemDataRow == nullptr) return false;

    if (ItemDataRow->MaterialAsset)
        ItemImage->SetBrushFromMaterial(ItemDataRow->MaterialAsset.Get());

    FString CleanNameString = ItemDataRow->Name.ToString();
    CleanNameString = CleanNameString.Replace(TEXT("\""), TEXT(""));
    ItemNameText->SetText(FText::FromString(CleanNameString));

    FString CleanDescriptionString = ItemDataRow->Description;
    CleanDescriptionString = CleanDescriptionString.Replace(TEXT("\""), TEXT(""));
    DescriptionText->SetText(FText::FromString(CleanDescriptionString));

    UEnum* EnumPtr = StaticEnum<EItemType>();
    FText DisplayText = EnumPtr->GetDisplayNameTextByValue((int64)ItemDataRow->ItemType);
    ItemTypeText->SetText(DisplayText);

    return true;
}

bool UAZTooltipWidget::UpdateToolTipInfo(int32 TargetID)
{
    if (TargetID == 0) return false;

    if (!ItemImage || !ItemNameText || !DescriptionText) return false;

    UWorld* World = GetWorld();
    if (World == nullptr) return false;

    UGameInstance* GI = World->GetGameInstance();
    if (GI == nullptr) return false;

    UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>();
    if (DataManager == nullptr) return false;

    const FAZBaseItemDataTable* ItemDataRow = DataManager->GetItemDataByID(TargetID);
    if (ItemDataRow == nullptr) return false;

    if (ItemDataRow->MaterialAsset)
        ItemImage->SetBrushFromMaterial(ItemDataRow->MaterialAsset.Get());

    FString CleanNameString = ItemDataRow->Name.ToString();
    CleanNameString = CleanNameString.Replace(TEXT("\""), TEXT(""));
    ItemNameText->SetText(FText::FromString(CleanNameString));

    FString CleanDescriptionString = ItemDataRow->Description;
    CleanDescriptionString = CleanDescriptionString.Replace(TEXT("\""), TEXT(""));
    DescriptionText->SetText(FText::FromString(CleanDescriptionString));

    UEnum* EnumPtr = StaticEnum<EItemType>();
    FText DisplayText = EnumPtr->GetDisplayNameTextByValue((int64)ItemDataRow->ItemType);
    ItemTypeText->SetText(DisplayText);

    return true;
}
