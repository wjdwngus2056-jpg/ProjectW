// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WAbility_Bolt.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NativeGameplayTags.h"
#include "ProjectW/AbilitySystem/Base/WAbilityContext.h"
#include "ProjectW/Actors/Projectiles/WBoltProjectile.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_WSkill_Type_Projectile, "Skill.Type.Projectile");

UWAbility_Bolt::UWAbility_Bolt()
{
	AbilityId = TEXT("Bolt");
	AbilityTags.AddTag(TAG_WSkill_Type_Projectile);
	BaseCooldownSeconds = 0.25f;
	ProjectileClass = AWBoltProjectile::StaticClass();
}

bool UWAbility_Bolt::CanActivateCustom(const FWAbilityContext& Context) const
{
	return ProjectileClass && !ProjectileClass->HasAnyClassFlags(CLASS_Abstract) &&
		FMath::IsFinite(Damage) && Damage >= 0.0f;
}

void UWAbility_Bolt::OnActivate(const FWAbilityContext& Context)
{
	AActor* Caster = GetAbilityOwner();
	UWorld* World = IsValid(Caster) ? Caster->GetWorld() : nullptr;
	const FVector Direction = Context.AimDirection.GetSafeNormal();
	if (!World || !ProjectileClass || ProjectileClass->HasAnyClassFlags(CLASS_Abstract) ||
		Direction.ContainsNaN() || Direction.IsNearlyZero())
	{
		CancelAbility(Context.ExecutionId);
		return;
	}

	// 시전자 중심에서 시작해 근거리 벽을 건너뛰지 않도록 합니다.
	const FTransform SpawnTransform(Direction.Rotation(), Caster->GetActorLocation());
	AWBoltProjectile* Projectile = World->SpawnActorDeferred<AWBoltProjectile>(
		ProjectileClass, SpawnTransform, Caster, Cast<APawn>(Caster),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!IsValid(Projectile))
	{
		CancelAbility(Context.ExecutionId);
		return;
	}

	// 생성 준비가 끝난 후 커밋하고, 성공한 실행만 월드에서 발사합니다.
	if (!CommitAbility(Context.ExecutionId))
	{
		Projectile->Destroy();
		return;
	}

	Projectile->SetDamage(Damage);
	Projectile->FinishSpawning(SpawnTransform);
	UE_LOG(LogWSkills, Verbose, TEXT("Bolt fired execution=%llu projectile=%s direction=%s"),
		Context.ExecutionId, *GetNameSafe(Projectile), *Direction.ToString());
	EndAbility(Context.ExecutionId);
}
