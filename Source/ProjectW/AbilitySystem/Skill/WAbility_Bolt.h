// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"
#include "ProjectW/AbilitySystem/Base/WAbility.h"
#include "WAbility_Bolt.generated.h"

class AWBoltProjectile;

/** 조준 방향으로 투사체 한 발을 발사하는 Ability */
UCLASS()
class PROJECTW_API UWAbility_Bolt : public UWAbility
{
	GENERATED_BODY()

public:
	UWAbility_Bolt();

protected:
	virtual bool CanActivateCustom(const FWAbilityContext& Context) const override;
	virtual void OnActivate(const FWAbilityContext& Context) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Bolt")
	TSubclassOf<AWBoltProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Bolt", meta = (ClampMin = "0.0"))
	float Damage = 10.0f;
};
