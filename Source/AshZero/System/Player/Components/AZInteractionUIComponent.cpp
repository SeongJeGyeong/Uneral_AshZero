// Fill out your copyright notice in the Description page of Project Settings.

#include "System/Player/Components/AZInteractionUIComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "UI/InteractionUI/AZInteractionWidget.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/AZInteractionComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Interface/AZInteractable.h"

UAZInteractionUIComponent::UAZInteractionUIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    bWantsInitializeComponent = true;

    WidgetScreenMargin = FMargin(100.f, 100.f, 100.f, 100.f);
}

void UAZInteractionUIComponent::BeginPlay()
{
    Super::BeginPlay();

    Owner = Cast<APlayerController>(GetOwner());
    if (!Owner || !Owner->IsLocalController()) return;

    if (Owner->GetPawn())
    {
        OnPawnChanged(nullptr, Owner->GetPawn());
    }

    Owner->OnPossessedPawnChanged.AddDynamic(this, &UAZInteractionUIComponent::OnPawnChanged);
}

void UAZInteractionUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Owner)
    {
        Owner->OnPossessedPawnChanged.RemoveDynamic(this, &UAZInteractionUIComponent::OnPawnChanged);
    }

    if (InteractionWidget)
    {
        InteractionWidget->RemoveFromParent();
        InteractionWidget = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

bool UAZInteractionUIComponent::IsWorldValid() const
{
    UWorld* World = GetWorld();
    return World && !World->bIsTearingDown;
}

void UAZInteractionUIComponent::ConstructMaterial()
{
    if (!IsWorldValid()) return;
    if (!Owner || !Owner->GetPawn()) return;

    PostProcessComponent = Cast<UPostProcessComponent>(
        Owner->GetPawn()->AddComponentByClass(UPostProcessComponent::StaticClass(), false, FTransform::Identity, false)
    );

    OutlineMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), MaterialInterface);

    if (PostProcessComponent && OutlineMaterial)
    {
        PostProcessComponent->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.f, OutlineMaterial));
    }
}

void UAZInteractionUIComponent::ConstructUI()
{
    if (!IsWorldValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("ConstructUI skipped - World is tearing down"));
        return;
    }

    if (!Owner || !Owner->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Warning, TEXT("ConstructUI skipped - Owner is invalid"));
        return;
    }

    if (!InteractionWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConstructUI skipped - InteractionWidgetClass is null"));
        return;
    }

    /*if (InteractionWidget)
    {
        return;
    }*/

    InteractionWidget = CreateWidget<UAZInteractionWidget>(Owner, InteractionWidgetClass);
    if (InteractionWidget)
    {
        InteractionWidget->AddToViewport(1);
        InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UAZInteractionUIComponent::DestroyUI()
{
    if (InteractionWidget)
    {
        InteractionWidget->RemoveFromParent();
        InteractionWidget = nullptr;
    }

    FocusTarget = nullptr;
    CurrentTargetDot = 0.f;
}

void UAZInteractionUIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!InteractionWidget) return;
    if (!IsWorldValid()) return;

    UpdateInteractable();
}

void UAZInteractionUIComponent::UpdateInteractable()
{
    if (!bIsVisible) return;

    AAZPlayerCharacter* Character = Cast<AAZPlayerCharacter>(Owner->GetPawn());
    if (Character)
    {
        AActor* FocusedActor = Character->InteractionComp->GetFocusedActor();
        if (FocusedActor)
        {
            if (FocusTarget == nullptr)
            {
                FocusTarget = FocusedActor;
                InteractionWidget->TargetActor = FocusedActor;
                TargetHighlighted(FocusTarget, true);
                TScriptInterface<IAZInteractable> InteractableTarget = TScriptInterface<IAZInteractable>(FocusTarget);
                InteractionWidget->SetInteractionText(InteractableTarget->InteractionKey, InteractableTarget->InteractionText);
            }
            else if (FocusTarget != FocusedActor)
            {
                TargetHighlighted(FocusTarget, false);
                TargetHighlighted(FocusedActor, true);
                FocusTarget = FocusedActor;
                InteractionWidget->TargetActor = FocusedActor;
                TScriptInterface<IAZInteractable> InteractableTarget = TScriptInterface<IAZInteractable>(FocusTarget);
                InteractionWidget->SetInteractionText(InteractableTarget->InteractionKey, InteractableTarget->InteractionText);
            }

            FVector CameraLocation = Owner->PlayerCameraManager->GetCameraLocation();
            FVector CameraForwardVector = Owner->PlayerCameraManager->GetActorForwardVector();
            FVector WidgetLocation = FocusTarget->GetActorLocation() - CameraLocation;
            WidgetLocation.Normalize();
            double TargetDot = WidgetLocation.Dot(CameraForwardVector);

            if (CurrentTargetDot > 0.5f && TargetDot > CurrentTargetDot)
            {
                CurrentTargetDot = TargetDot;
            }

            InteractionWidget->UpdatePositionInViewport(WidgetScreenMargin, ScreenRadiusPercent);
            InteractionWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            if (FocusTarget)
            {
                TargetHighlighted(FocusTarget, false);
                FocusTarget = nullptr;
            }

            InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UAZInteractionUIComponent::TargetHighlighted(AActor* Target, bool bIsHighlighted)
{
    if (!Target || !OutlineMaterial) return;

    TScriptInterface<IAZInteractable> InteractableTarget = TScriptInterface<IAZInteractable>(Target);
    FLinearColor HighlightColor = SwitchIntToColor(static_cast<uint32>(InteractableTarget->HighlightColor));
    OutlineMaterial->SetVectorParameterValue(FName("Outline Color"), HighlightColor);

    TArray<USceneComponent*> Children;
    Target->GetRootComponent()->GetChildrenComponents(true, Children);

    for (USceneComponent* Child : Children)
    {
        if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Child))
        {
            Primitive->SetRenderCustomDepth(bIsHighlighted);
        }
    }
}

FLinearColor UAZInteractionUIComponent::SwitchIntToColor(uint32 Color)
{
    float R = ((Color >> 16) & 0xFF) / 255.0f;
    float G = ((Color >> 8) & 0xFF) / 255.0f;
    float B = (Color & 0xFF) / 255.0f;

    return FLinearColor(R, G, B, 1.f);
}

void UAZInteractionUIComponent::SetVisibleState(ESlateVisibility Visibility)
{
    bIsVisible = (Visibility == ESlateVisibility::Collapsed || Visibility == ESlateVisibility::Hidden);
    if (!bIsVisible) InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UAZInteractionUIComponent::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
    if (!IsWorldValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPawnChanged skipped - World is tearing down"));
        return;
    }

    if (!Owner || !Owner->IsLocalController()) return;

    if (!NewPawn)
    {
        DestroyUI();
        SetComponentTickEnabled(false);
        return;
    }

    ConstructMaterial();
    ConstructUI();

    SetComponentTickEnabled(true);
}