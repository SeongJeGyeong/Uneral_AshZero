// IAZStatusInterface.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AZStatusTypes.h"
#include "AZStatusInterface.generated.h"

UINTERFACE(MinimalAPI)
class UAZStatusInterface : public UInterface { GENERATED_BODY() };

class ASHZERO_API IAZStatusInterface
{
	GENERATED_BODY()

public:
	/** * 상태 이상 패킷을 전달받는 공용 함수
	 * @param Packet : 전달할 상태 정보
	 * @param Causer : 효과를 발생시킨 원인 (무기 주인, 장판 소유자 등)
	 */
	virtual void ApplyStatusEffect(const FAZStatusEffectPacket& Packet, AActor* Causer = nullptr) = 0;
};