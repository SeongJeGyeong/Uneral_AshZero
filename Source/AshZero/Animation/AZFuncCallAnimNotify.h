// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Util/AZDefine.h"
#include "AZFuncCallAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class ASHZERO_API UAZFuncCallAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Call")
	UClass* TargetClass;

	// 에디터에서 콤보박스로 보여줄 함수 이름GetCallableFunctionNames
	UPROPERTY(EditAnywhere, Category = "Call", meta = (GetOptions = "GetCallableFunctionNames"))
	FName FunctionName;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	UFUNCTION()
	TArray<FName> GetCallableFunctionNames() const;

private:
	// 캐시(FindFunction 비용 제거)
	UFunction* CachedFunction = nullptr;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void ValidateAssociatedAssets() override; // 경고 표시
#endif
};
