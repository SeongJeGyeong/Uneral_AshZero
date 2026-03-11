// Fill out your copyright notice in the Description page of Project Settings.


#include "AZRoomDataActionUtility.h"
#include "Levels/Rooms/AZBaseRoom.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "FileHelpers.h"
#include "Misc/FeedbackContext.h" // 진행바 표시용
#include "AshZero/DataAsset/AZRoomDataAsset.h"
#include "EditorUtilityLibrary.h"

void UAZRoomDataActionUtility::UpdateRoomDataFromLevel()
{
#if WITH_EDITOR
    TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
    UAZRoomDataAsset* TargetAsset = nullptr;

    for (UObject* Asset : SelectedAssets)
    {
        if (UAZRoomDataAsset* CastedAsset = Cast<UAZRoomDataAsset>(Asset))
        {
            TargetAsset = CastedAsset;
            break;
        }
    }

    if (!TargetAsset) return;

    // 현재 작업 중인 맵 저장 (날아감 방지)
    if (!FEditorFileUtils::SaveDirtyPackages(true, true, true))
    {
        return; // 사용자가 저장을 취소하면 중단
    }

    // 원본 맵 경로
    UWorld* CurrentWorld = GEditor->GetEditorWorldContext().World();
    FString OriginalMapPackageName = CurrentWorld ? CurrentWorld->GetOutermost()->GetName() : TEXT("");

    // 총 작업량(분모), 메시지, 활성화 여부
    FScopedSlowTask SlowTask(TargetAsset->RoomDataList.Num(), FText::FromString(TEXT("Updating Room Data...")), true);
    SlowTask.MakeDialog(true); // 취소 버튼 활성화
    
    int32 TotalUpdatedRooms = 0;
    TargetAsset->Modify();
    for (FSpawnLevelData& LevelData : TargetAsset->RoomDataList)
    {
        // 취소 버튼 눌렀는지 체크
        if (SlowTask.ShouldCancel()) break;

        FRoomData& RoomData = LevelData.RoomData;

        // 레벨 에셋이 비어있으면 스킵
        if (RoomData.LevelAsset.IsNull()) continue;

        FString LevelPackageName = RoomData.LevelAsset.GetLongPackageName();
        SlowTask.EnterProgressFrame(1.0f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *LevelPackageName)));

        if (UpdateRoomData(RoomData, LevelPackageName))
        {
            TotalUpdatedRooms++;
            UE_LOG(LogTemp, Log, TEXT("Updated Room: %s"), *LevelPackageName);
        }        
    }

    FSpawnLevelData& BossLevelData = TargetAsset->BossRoomData;
    FRoomData& RoomData = BossLevelData.RoomData;

    // 레벨 에셋이 비어있으면 스킵
    if (!RoomData.LevelAsset.IsNull())
    {
        FString LevelPackageName = RoomData.LevelAsset.GetLongPackageName();
        SlowTask.EnterProgressFrame(1.0f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *LevelPackageName)));

        if (UpdateRoomData(RoomData, LevelPackageName))
        {
            TotalUpdatedRooms++;
            UE_LOG(LogTemp, Log, TEXT("Updated Room: %s"), *LevelPackageName);
        }
    }

    TargetAsset->MarkPackageDirty();

    // 원래 맵으로 복귀
    if (!OriginalMapPackageName.IsEmpty())
    {
        FEditorFileUtils::LoadMap(OriginalMapPackageName, false, true);
    }

    // 결과 알림
    FText ResultMsg = FText::Format(FText::FromString(TEXT("Successfully updated {0} assets.")), TotalUpdatedRooms);
    FMessageDialog::Open(EAppMsgType::Ok, ResultMsg);
#endif
}

bool UAZRoomDataActionUtility::UpdateRoomData(FRoomData& RoomData, const FString& LevelPackageName)
{
    if (!FEditorFileUtils::LoadMap(LevelPackageName, false, true)) return false;

    UWorld* LoadedWorld = GEditor->GetEditorWorldContext().World();
    AActor* FoundActor = UGameplayStatics::GetActorOfClass(LoadedWorld, AAZBaseRoom::StaticClass());
    AAZBaseRoom* RoomActor = Cast<AAZBaseRoom>(FoundActor);
    if (!RoomActor) return false;

    if (RoomActor->OverlapTrigger)
    {
        RoomData.BoundsExtent = RoomActor->OverlapTrigger->GetScaledBoxExtent();
        RoomData.BoundsOffset = RoomActor->OverlapTrigger->GetRelativeLocation();
    }

    if (RoomActor->ExitPoints)
    {
        RoomData.LocalExitTransforms.Empty();
        TArray<USceneComponent*> Children;
        RoomActor->ExitPoints->GetChildrenComponents(false, Children);

        for (USceneComponent* ChildComp : Children)
        {
            FTransform RelativeToActor = 
                ChildComp->GetComponentTransform().GetRelativeTransform(RoomActor->GetActorTransform());
            RoomData.LocalExitTransforms.Add(RelativeToActor);
        }
    }

    return true;
}
