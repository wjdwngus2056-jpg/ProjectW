// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectW/Actors/Interfaces/WInteractable.h"
#include "WMissionPanel.generated.h"

class UBoxComponent;

/** 상호작용하면 지정된 미션의 선택을 거점 GameMode에 요청하는 패널. */
UCLASS()
class PROJECTW_API AWMissionPanel : public AActor, public IWInteractable
{
	GENERATED_BODY()

public:
	AWMissionPanel();

	virtual bool CanInteract(AActor* Interactor) const override;
	virtual bool TryInteract(AActor* Interactor) override;
	virtual FText GetInteractionText(AActor* Interactor) const override;

protected:
	/** 플레이어의 상호작용 후보 탐색에 사용되는 충돌 영역. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UBoxComponent> InteractionBox;

	/** GameMode의 미션 DataTable에서 선택할 행 이름. 패널 인스턴스마다 지정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName MissionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0"))
	float InteractionDistance = 200.0f;
};
