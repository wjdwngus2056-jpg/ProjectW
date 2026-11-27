// Copyright (C) 2026 TeamW. All Rights Reserved.

#include "ProjectW/AbilitySystem/Skill/WAbility_Bolt.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "EngineUtils.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "ProjectW/Actors/Player/WPlayerCharacter.h"
#include "ProjectW/Actors/Collision/WCollisionChannels.h"
#include "ProjectW/Actors/Projectiles/WBoltProjectile.h"
#include "ProjectW/Core/Framework/WPlayerController.h"
#include "WAbilityTestTypes.h"
#include "WSkillTestWorld.h"

namespace
{
	int32 CountAssetTestBolts(UWorld* World)
	{
		int32 Count = 0;
		for (TActorIterator<AWBoltProjectile> It(World); It; ++It)
		{
			++Count;
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWSkillAssetWiringTest, "ProjectW.Skills.AssetWiring",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWSkillAssetWiringTest::RunTest(const FString& Parameters)
{
	UClass* CharacterClass = LoadClass<AWPlayerCharacter>(nullptr, TEXT("/Game/Actors/Character/BP_PlayerCharacter.BP_PlayerCharacter_C"));
	UClass* ControllerClass = LoadClass<AWPlayerController>(nullptr, TEXT("/Game/Core/Framework/PlayerController/BP_WPlayerController.BP_WPlayerController_C"));
	UClass* AbilityClass = LoadClass<UWAbility_Bolt>(nullptr, TEXT("/Game/AbilitySystem/Bolts/AS_Test_Bolt.AS_Test_Bolt_C"));
	UClass* ProjectileClass = LoadClass<AWBoltProjectile>(nullptr, TEXT("/Game/Actors/Projectile/BP_Test_Bolt.BP_Test_Bolt_C"));
	UInputAction* Action = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/InputAction/IA_W_BasicAttack.IA_W_BasicAttack"));
	UInputMappingContext* Mapping = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_MainIMC.IMC_MainIMC"));
	if (!TestNotNull(TEXT("Character BP"), CharacterClass) || !TestNotNull(TEXT("Controller BP"), ControllerClass) ||
		!TestNotNull(TEXT("Ability BP"), AbilityClass) || !TestNotNull(TEXT("Projectile BP"), ProjectileClass) ||
		!TestNotNull(TEXT("Input action"), Action) || !TestNotNull(TEXT("Mapping context"), Mapping))
	{
		return false;
	}
	const AWPlayerController* Controller = ControllerClass->GetDefaultObject<AWPlayerController>();
	TestEqual(TEXT("Controller uses basic action"), Controller->BasicAttackAction.Get(), Action);
	TestEqual(TEXT("Controller uses mapping context"), Controller->MainMappingContext.Get(), Mapping);
	TestEqual(TEXT("Basic action is Boolean"), Action->ValueType, EInputActionValueType::Boolean);
	int32 LeftClickMappings = 0;
	for (const FEnhancedActionKeyMapping& KeyMapping : Mapping->GetMappings())
	{
		if (KeyMapping.Key == EKeys::LeftMouseButton && KeyMapping.Action == Action)
		{
			++LeftClickMappings;
		}
	}
	TestEqual(TEXT("Exactly one left-click basic mapping"), LeftClickMappings, 1);
	const UWAbility* Definition = AbilityClass->GetDefaultObject<UWAbility>();
	TestTrue(TEXT("Projectile classification tag"), Definition->GetAbilityTags().HasTagExact(
		FGameplayTag::RequestGameplayTag(TEXT("Skill.Type.Projectile"))));
	const FClassProperty* ProjectileProperty = FindFProperty<FClassProperty>(AbilityClass, TEXT("ProjectileClass"));
	TestEqual(TEXT("Ability BP selects projectile BP"), ProjectileProperty->GetObjectPropertyValue_InContainer(Definition), static_cast<UObject*>(ProjectileClass));

	FWSkillTestWorld TestWorld;
	AWPlayerCharacter* Character = TestWorld.World->SpawnActor<AWPlayerCharacter>(CharacterClass, FVector(0.0, 0.0, 100.0), FRotator::ZeroRotator);
	UWSkillComponent* Skills = Character->FindComponentByClass<UWSkillComponent>();
	if (!TestNotNull(TEXT("Character skill component"), Skills))
	{
		return false;
	}
	UWAbility* Equipped = Skills->GetEquippedAbility(EWAbilitySlot::BasicAttack);
	if (!TestNotNull(TEXT("Character equips ability BP"), Equipped))
	{
		return false;
	}
	TestEqual(TEXT("Configured ability class"), Equipped->GetClass(), AbilityClass);
	TestEqual(TEXT("Player channel config matches C++"), UCollisionProfile::Get()->ReturnChannelNameFromContainerIndex(WCollisionChannels::PlayerCharacter), FName(TEXT("PlayerCharacter")));
	TestEqual(TEXT("Projectile channel config matches C++"), UCollisionProfile::Get()->ReturnChannelNameFromContainerIndex(WCollisionChannels::SkillProjectile), FName(TEXT("SkillProjectile")));
	TestEqual(TEXT("BP capsule uses player channel"), Character->GetCapsuleComponent()->GetCollisionObjectType(), WCollisionChannels::PlayerCharacter);
	TestEqual(TEXT("BP mesh uses player channel"), Character->GetMesh()->GetCollisionObjectType(), WCollisionChannels::PlayerCharacter);
	TestEqual(TEXT("Player ignores projectile channel"), Character->GetCapsuleComponent()->GetCollisionResponseToChannel(WCollisionChannels::SkillProjectile), ECR_Ignore);
	Character->GetCharacterMovement()->bRunPhysicsWithNoController = true;
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	const FVector BeforeShot = Character->GetActorLocation();
	TestTrue(TEXT("Real asset slot launches"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).IsAccepted());
	TestEqual(TEXT("Real asset spawns one Bolt"), CountAssetTestBolts(TestWorld.World), 1);
	for (TActorIterator<AWBoltProjectile> It(TestWorld.World); It; ++It)
	{
		TestEqual(TEXT("Spawned projectile class"), It->GetClass(), ProjectileClass);
		TestEqual(TEXT("Projectile BP collision channel"), It->FindComponentByClass<USphereComponent>()->GetCollisionObjectType(), WCollisionChannels::SkillProjectile);
		TestEqual(TEXT("Projectile BP visual mesh has no collision"), It->FindComponentByClass<UStaticMeshComponent>()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	}
	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	TestFalse(TEXT("Real BP Bolt does not block player capsule"), TestWorld.World->OverlapBlockingTestByChannel(
		Capsule->GetComponentLocation(), Capsule->GetComponentQuat(), Capsule->GetCollisionObjectType(), Capsule->GetCollisionShape(),
		FCollisionQueryParams(SCENE_QUERY_STAT(BoltAssetOverlap), false, Character), FCollisionResponseParams(Capsule->GetCollisionResponseToChannels())));
	TestWorld.Advance(1.0f / 60.0f);
	TestTrue(TEXT("Firing does not move real BP character"), Character->GetActorLocation().Equals(BeforeShot, 0.1));
	TestEqual(TEXT("Shared asset definition remains idle"), Definition->GetState(), EWAbilityState::Idle);
	AWAbilityDamageTestTarget* Target = TestWorld.World->SpawnActor<AWAbilityDamageTestTarget>(
		FVector(250.0, 0.0, 100.0), FRotator::ZeroRotator);
	TestWorld.Advance(0.4f);
	TestEqual(TEXT("Projectile BP hits once"), Target->HitCount, 1);
	TestEqual(TEXT("Projectile BP ends on impact"), CountAssetTestBolts(TestWorld.World), 0);
	TestTrue(TEXT("Real asset can fire again"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::RightVector).IsAccepted());
	TestWorld.Advance(2.2f);
	TestEqual(TEXT("Projectile BP expires on miss"), CountAssetTestBolts(TestWorld.World), 0);
	return true;
}
#endif
