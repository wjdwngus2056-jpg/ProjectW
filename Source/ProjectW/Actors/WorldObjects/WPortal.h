// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WPortal.generated.h"

UCLASS()
class PROJECTW_API AWPortal : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWPortal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
