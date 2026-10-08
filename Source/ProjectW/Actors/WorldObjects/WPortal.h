// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectW/Actors/Interfaces/WInteractable.h"
#include "WPortal.generated.h"

UCLASS()
class PROJECTW_API AWPortal : public AActor, public IWInteractable
{
	GENERATED_BODY()

public:
	AWPortal();
	
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual bool TryInteract(AActor* Interactor) override;
	virtual FText GetInteractionText(AActor* Interactor) const override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0"))
	float InteractionDistance = 200.0f;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Interaction")
	bool bHasInteracted = false;
};
