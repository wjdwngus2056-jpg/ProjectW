// Copyright (C) 2026 TeamW. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectW/Actors/Interfaces/WInteractable.h"
#include "WRescueTarget.generated.h"

class UWMissionObjectiveComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

/** 시민 1명의 즉시 구조. 상태는 소유 목표 컴포넌트, 접근·구조 조건은 이 Actor가 담당한다. */
UCLASS()
class PROJECTW_API AWRescueTarget : public AActor, public IWInteractable
{
	GENERATED_BODY()
public:
	AWRescueTarget();
	/** 유효한 같은 월드의 플레이어가 거리 안에 있고 미구조일 때만 가능하다. */
	virtual bool CanInteract(AActor* Interactor) const override;
	/** 실행 직전 재검증하고 최초 구조를 보고한다. 결과 확정·이동·HUD는 호출하지 않는다. */
	virtual bool TryInteract(AActor* Interactor) override;
	virtual FText GetInteractionText(AActor* Interactor) const override;
	UFUNCTION(BlueprintPure, Category = "Rescue") bool IsRescued() const;
protected:
	virtual void BeginPlay() override;
private:
	/** 도형·SOS/OK는 구조 상태 확인용 임시 표현이며 정식 시민 모델·피드백으로 교체한다. */
	void UpdateRescueVisuals();
	UPROPERTY(VisibleAnywhere, Category = "Rescue") TObjectPtr<UWMissionObjectiveComponent> Objective;
	UPROPERTY(VisibleAnywhere, Category = "Rescue") TObjectPtr<UStaticMeshComponent> Body;
	UPROPERTY(VisibleAnywhere, Category = "Rescue") TObjectPtr<UStaticMeshComponent> Head;
	UPROPERTY(VisibleAnywhere, Category = "Rescue") TObjectPtr<UTextRenderComponent> StatusText;
	UPROPERTY(EditAnywhere, Category = "Rescue", meta = (ClampMin = "1")) float InteractionDistance = 200.0f;
};
