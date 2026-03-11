
#include "Components/AZPlayerBaseComponent.h"
#include "Character/AZPlayerCharacter.h"
#include "AshZero.h"
UAZPlayerBaseComponent::UAZPlayerBaseComponent()
{
	bWantsInitializeComponent = true;
}

void UAZPlayerBaseComponent::InitializeComponent()
{
	Super::InitializeComponent();
	OwnerCharacter = Cast<AAZPlayerCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		PRINT_LOG(TEXT("OwnerCharacter is NULL!"));
		return;
	}
	CharacterMoveComp = OwnerCharacter->GetCharacterMovement();
	PRINT_LOG(TEXT("BaseComponent Initialized!"));
	// 델리게이트에 처리 함수 등록
	OwnerCharacter->OnInputBindingDelegate.AddUObject(this, &UAZPlayerBaseComponent::SetupInputBinding);
}

// Called when the game starts
void UAZPlayerBaseComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}