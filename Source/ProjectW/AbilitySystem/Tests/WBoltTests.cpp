// Copyright (C) 2026 TeamW. All Rights Reserved.

#include "ProjectW/AbilitySystem/Skill/WAbility_Bolt.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ProjectW/Actors/Player/WPlayerCharacter.h"
#include "ProjectW/Actors/Projectiles/WBoltProjectile.h"
#include "WAbilityTestTypes.h"
#include "WSkillTestWorld.h"

namespace
{
	int32 CountBolts(UWorld* World)
	{
		int32 Count = 0;
		for (TActorIterator<AWBoltProjectile> It(World); It; ++It)
		{
			++Count;
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWBoltProjectileTest, "ProjectW.Skills.BoltProjectile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWBoltProjectileTest::RunTest(const FString& Parameters)
{
	FWSkillTestWorld TestWorld;
	UWSkillComponent* Skills = TestWorld.AddSkills(UWAbility_Bolt::StaticClass());
	UWAbility* Ability = Skills->GetEquippedAbility(EWAbilitySlot::BasicAttack);
	AWAbilityDamageTestTarget* Target = TestWorld.World->SpawnActor<AWAbilityDamageTestTarget>(
		FVector(250.0, 0.0, 100.0), FRotator::ZeroRotator);
	int32 EndCount = 0;
	const FDelegateHandle EndHandle = Skills->OnAbilityEnded.AddLambda([this, &EndCount](FName, uint64, bool bCancelled)
	{
		++EndCount;
		TestFalse(TEXT("Bolt ends normally"), bCancelled);
	});
	const auto Shot = Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector);
	TestTrue(TEXT("Bolt request accepted"), Shot.IsAccepted());
	TestEqual(TEXT("Exactly one projectile"), CountBolts(TestWorld.World), 1);
	TestEqual(TEXT("Ability ends immediately after launch"), Ability->GetState(), EWAbilityState::Idle);
	TestEqual(TEXT("One normal end notification"), EndCount, 1);
	TestEqual(TEXT("Rapid repeat rejected"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).Status, EWAbilityRequestStatus::Cooldown);
	TestWorld.Advance(0.1f);
	TestEqual(TEXT("Projectile ignores caster collision"), CountBolts(TestWorld.World), 1);
	TestWorld.Advance(0.3f);
	TestEqual(TEXT("Blocking impact destroys projectile"), CountBolts(TestWorld.World), 0);
	TestEqual(TEXT("Impact damage occurs once"), Target->HitCount, 1);
	TestEqual(TEXT("Configured damage applied"), Target->TotalDamage, 10.0f);
	UPrimitiveComponent* TargetCollision = CastChecked<UPrimitiveComponent>(Target->GetRootComponent());
	TargetCollision->SetCollisionObjectType(ECC_Pawn);
	TestTrue(TEXT("Shot against Pawn accepted"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).IsAccepted());
	TestWorld.Advance(0.4f);
	TestEqual(TEXT("Enemy Pawn collision retained"), Target->HitCount, 2);
	TargetCollision->SetCollisionObjectType(ECC_WorldStatic);
	TestTrue(TEXT("Shot against world geometry accepted"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).IsAccepted());
	TestWorld.Advance(0.4f);
	TestEqual(TEXT("WorldStatic collision retained"), Target->HitCount, 3);

	TestTrue(TEXT("Second shot after cooldown"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::RightVector).IsAccepted());
	TestWorld.Advance(2.2f);
	TestEqual(TEXT("Miss expires by lifespan"), CountBolts(TestWorld.World), 0);
	TestEqual(TEXT("Miss does not damage target"), Target->HitCount, 3);

	FWSkillTestWorld::SetClass(Ability, TEXT("ProjectileClass"), nullptr);
	TestEqual(TEXT("Invalid projectile class rejected before commit"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).Status, EWAbilityRequestStatus::CustomConditionFailed);
	TestEqual(TEXT("Invalid projectile config has no cooldown"), Skills->GetCooldownRemaining(Ability->GetAbilityId()), 0.0f);
	TestEqual(TEXT("Invalid projectile config spawns nothing"), CountBolts(TestWorld.World), 0);
	Skills->OnAbilityEnded.Remove(EndHandle);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWBoltPlayerCollisionTest, "ProjectW.Skills.BoltDoesNotPushPlayer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWBoltPlayerCollisionTest::RunTest(const FString& Parameters)
{
	FWSkillTestWorld TestWorld;
	const FTransform Transform(FVector(0.0, 0.0, 100.0));
	AWPlayerCharacter* Character = TestWorld.World->SpawnActorDeferred<AWPlayerCharacter>(
		AWPlayerCharacter::StaticClass(), Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	UWSkillComponent* Skills = Character->FindComponentByClass<UWSkillComponent>();
	FWSkillTestWorld::SetClass(Skills, TEXT("BasicAbilityClass"), UWAbility_Bolt::StaticClass());
	Character->FinishSpawning(Transform);
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	Movement->bRunPhysicsWithNoController = true;
	Movement->SetMovementMode(MOVE_Flying);
	const FVector Directions[] = { FVector::ForwardVector, FVector::RightVector, -FVector::ForwardVector, -FVector::RightVector };
	for (const FVector& Direction : Directions)
	{
		const FVector Before = Character->GetActorLocation();
		TestTrue(TEXT("Shot accepted"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, Direction).IsAccepted());
		const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BoltPlayerOverlap), false, Character);
		TestFalse(TEXT("Spawned Bolt must not be a blocking overlap for the player capsule"),
			TestWorld.World->OverlapBlockingTestByChannel(Capsule->GetComponentLocation(), Capsule->GetComponentQuat(),
				Capsule->GetCollisionObjectType(), Capsule->GetCollisionShape(), QueryParams,
				FCollisionResponseParams(Capsule->GetCollisionResponseToChannels())));
		// 발사 직후 작은 이동 스윕에서도 투사체와 겹침 보정이 일어나면 안 됩니다.
		Character->AddMovementInput(-Direction);
		TestWorld.Advance(1.0f / 60.0f);
		const double Distance = FVector::Dist2D(Before, Character->GetActorLocation());
		TestTrue(FString::Printf(TEXT("Shot must not depenetrate player (distance=%.3f)"), Distance), Distance < 2.0);
		Movement->StopMovementImmediately();
		TestWorld.Advance(0.3f);
	}
	return true;
}
#endif
