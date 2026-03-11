
#include "AZFuncCallAnimNotify.h"


TArray<FName> UAZFuncCallAnimNotify::GetCallableFunctionNames() const
{
    TArray<FName> Out;

    if (TargetClass)
    {
        //UClass* TargetClass = UAnimInstance::StaticClass();
        for (TFieldIterator<UFunction> It(TargetClass, EFieldIteratorFlags::ExcludeSuper); It; ++It)
        {
            UFunction* Fn = *It;
            const bool bCallable = Fn->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_BlueprintEvent);
            if (!bCallable) continue;

            Out.Add(Fn->GetFName());
        }
    }

    Out.Sort(FNameLexicalLess());
    return Out;
}

void UAZFuncCallAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    if (!TargetClass) return;

    UObject* Target = nullptr;
    AActor* OwnerActor = MeshComp->GetOwner();
    // 애니메이션 인스턴스 체크
    if (TargetClass->IsChildOf<UAnimInstance>())
    {
        Target = Cast<UObject>(MeshComp->GetAnimInstance());
    }
    // 액터 컴포넌트인지 체크
    else if (TargetClass->IsChildOf<UActorComponent>())
    {
        // 애니메이션이 재생중인 스켈레탈 메시의 부모 액터
        if (OwnerActor)
        {
            Target = OwnerActor->FindComponentByClass(TargetClass); // 해당 컴포넌트 타입을 찾아온다.
        }
    }
    // 디폴트로 액터의 함수를 찾는다
    else
    {
        Target = MeshComp->GetOwner();
    }

    if (!Target) return;

    // 캐시 없으면 한 번만 탐색
    if (CachedFunction == nullptr || CachedFunction->GetFName() != FunctionName)
    {
        CachedFunction = Target->FindFunction(FunctionName);
    }

    if (CachedFunction == nullptr)
    {
        CachedFunction = nullptr;
        return;
    }

    // 애니메이션 블루프린트 -> 노티파이 이벤트 -> 이벤트에서 FireComponent 찾아서 연결
    // 함수 호출
    Target->ProcessEvent(CachedFunction, nullptr);
}

#if WITH_EDITOR
void UAZFuncCallAnimNotify::PostEditChangeProperty(FPropertyChangedEvent& E)
{
    Super::PostEditChangeProperty(E);

    if (E.Property && E.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAZFuncCallAnimNotify, FunctionName))
    {
        CachedFunction = nullptr;
    }
}

void UAZFuncCallAnimNotify::ValidateAssociatedAssets()
{
    Super::ValidateAssociatedAssets();
    if (FunctionName.IsNone())
    {

    }
}
#endif