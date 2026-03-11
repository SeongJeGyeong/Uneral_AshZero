// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "NativeGameplayTags.h"

// 가독성을 위해서 네임스페이스로 감싼다
namespace AZGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Look);	//2개로 분리된사람은 두개로 선언
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Run);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Jump);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Interact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Reload);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Roll);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Aim);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Inventory);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Num1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Num2);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Num3);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Num4);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_SwitchWeapon);

	// ===== Boss Pattern Tags =====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack2);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack3);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack4);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack5);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack6);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack7);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack8);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack8);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack9);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack10);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_Action_Attack11);

};

