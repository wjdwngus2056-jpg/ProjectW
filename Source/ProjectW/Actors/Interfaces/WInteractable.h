// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WInteractable.generated.h"

class AActor;

UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class PROJECTW_API UWInteractable : public UInterface
{
	GENERATED_BODY()
};

/** 미션 입구, NPC, 월드 오브젝트같은 상호작용 가능한 오브젝트들이 공유하는 인터페이스. */
class PROJECTW_API IWInteractable
{
	GENERATED_BODY()

public:
	/** 상호작용 가능 여부를 조회 */
	virtual bool CanInteract(AActor* Interactor) const = 0;

	/**
	 * 상호작용을 시도하고 요청이 수락되었는지 반환.
	 * 이전 CanInteract 결과가 실제 실행 가능 여부를 보장하지는 않도록 함.
	 */
	virtual bool TryInteract(AActor* Interactor) = 0;

	/** 입력 키를 제외한 행동 문구를 반환한다. ex) "출동", "구출", "대화" */
	virtual FText GetInteractionText(AActor* Interactor) const = 0;
};
