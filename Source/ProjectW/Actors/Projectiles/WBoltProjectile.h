// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WBoltProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

/** Bolt 계열 스킬이 발사하는 직선 투사체 */
UCLASS()
class PROJECTW_API AWBoltProjectile : public AActor
{
	GENERATED_BODY()

public:
	AWBoltProjectile();

	void SetDamage(float InDamage);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnProjectileStopped(const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, Category = "Bolt")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Bolt")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Bolt")
	TObjectPtr<UProjectileMovementComponent> Movement;

	float Damage = 0.0f;
	bool bHasImpacted = false;
};
