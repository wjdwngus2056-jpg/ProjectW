// Copyright (C) 2026 TeamW. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectW/Actors/Interfaces/WInteractable.h"
#include "WDevelopmentMissionObjective.generated.h"

/** 왕복 검증 전용. 실제 구조·처치·진압 콘텐츠로 간주하지 않는다. */
UCLASS()
class PROJECTW_API AWDevelopmentMissionObjective : public AActor, public IWInteractable
{
	GENERATED_BODY()
public:
	AWDevelopmentMissionObjective();
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual bool TryInteract(AActor* Interactor) override;
	virtual FText GetInteractionText(AActor* Interactor) const override;
	FName GetObjectiveId() const { return ObjectiveId; }
private:
	UPROPERTY(VisibleAnywhere) TObjectPtr<class UStaticMeshComponent> Mesh;
	UPROPERTY(EditAnywhere, Category = "Mission|Development") FName ObjectiveId = TEXT("PrototypeComplete");
};
