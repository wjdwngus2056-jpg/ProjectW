// Copyright (C) 2026 TeamW. All Rights Reserved.

#include "WAbilityTestTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "WSkillTestWorld.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAbilityLifecycleTest, "ProjectW.Skills.LifecycleAndStaleExecution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWAbilityLifecycleTest::RunTest(const FString& Parameters)
{
	FWSkillTestWorld TestWorld;
	UWSkillComponent* Skills = TestWorld.AddSkills(UWAbilityLifecycleTestProbe::StaticClass());
	UWAbilityLifecycleTestProbe* Ability = CastChecked<UWAbilityLifecycleTestProbe>(Skills->GetEquippedAbility(EWAbilitySlot::BasicAttack));
	TestEqual(TEXT("Empty special slot"), Skills->TryActivateSlot(EWAbilitySlot::SpecialAttack, FVector::ForwardVector).Status, EWAbilityRequestStatus::EmptySlot);
	TestEqual(TEXT("Zero aim rejected"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ZeroVector).Status, EWAbilityRequestStatus::InvalidAim);
	FVector InvalidAim = FVector::ForwardVector;
	InvalidAim.X = std::numeric_limits<double>::infinity();
	TestEqual(TEXT("Infinite aim rejected before normalization"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, InvalidAim).Status, EWAbilityRequestStatus::InvalidAim);

	const FWAbilityRequestResult First = Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector(2.0, 0.0, 0.0));
	TestTrue(TEXT("Accepted with nonzero ID"), First.IsAccepted() && First.ExecutionId != 0);
	TestEqual(TEXT("Preparing"), Ability->GetState(), EWAbilityState::Preparing);
	TestEqual(TEXT("Overlap rejected"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).Status, EWAbilityRequestStatus::AlreadyActive);
	TestTrue(TEXT("Commit succeeds"), Ability->Commit(First.ExecutionId));
	TestWorld.Advance(0.2f);
	const float Remaining = Skills->GetCooldownRemaining(Ability->GetAbilityId());
	TestTrue(TEXT("Repeat commit is idempotent"), Ability->Commit(First.ExecutionId));
	TestEqual(TEXT("Repeat commit does not restart cooldown"), Skills->GetCooldownRemaining(Ability->GetAbilityId()), Remaining);
	Ability->Finish(First.ExecutionId);
	Ability->Cancel(First.ExecutionId);
	TestEqual(TEXT("End hook called once"), Ability->EndCount, 1);
	TestFalse(TEXT("Normal end"), Ability->bLastCancelled);
	TestEqual(TEXT("Cooldown rejects new request"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).Status, EWAbilityRequestStatus::Cooldown);
	TestWorld.Advance(1.0f);

	const FWAbilityRequestResult Next = Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector);
	TestTrue(TEXT("New execution after cooldown"), Next.IsAccepted() && Next.ExecutionId != First.ExecutionId);
	Skills->ReleaseSlot(EWAbilitySlot::BasicAttack, First.ExecutionId);
	Ability->Cancel(First.ExecutionId);
	TestFalse(TEXT("Old commit rejected"), Ability->Commit(First.ExecutionId));
	TestEqual(TEXT("Old callbacks preserve new execution"), Ability->GetActiveExecutionId(), Next.ExecutionId);
	Skills->ReleaseSlot(EWAbilitySlot::BasicAttack, Next.ExecutionId);
	TestEqual(TEXT("Matching release ends execution"), Ability->GetState(), EWAbilityState::Idle);
	TestEqual(TEXT("Matching release ends once"), Ability->EndCount, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAbilityCleanupTest, "ProjectW.Skills.CommitRevalidationAndCleanup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWAbilityCleanupTest::RunTest(const FString& Parameters)
{
	FWSkillTestWorld TestWorld;
	UWSkillComponent* Skills = TestWorld.AddSkills(UWAbilityLifecycleTestProbe::StaticClass());
	UWAbilityLifecycleTestProbe* Ability = CastChecked<UWAbilityLifecycleTestProbe>(Skills->GetEquippedAbility(EWAbilitySlot::BasicAttack));
	const auto Request = Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector);
	Ability->bCondition = false;
	TestFalse(TEXT("Changed condition prevents commit"), Ability->Commit(Request.ExecutionId));
	TestTrue(TEXT("Failed commit cancels"), Ability->bLastCancelled);
	TestEqual(TEXT("Failed commit has no cooldown"), Skills->GetCooldownRemaining(Ability->GetAbilityId()), 0.0f);
	TestWorld.Advance(0.2f);
	TestEqual(TEXT("Cancelled timer never runs"), Ability->TimerCount, 0);

	Ability->bCondition = true;
	Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector);
	const FDelegateHandle EndHandle = Skills->OnAbilityEnded.AddLambda([this, Skills](FName, uint64, bool)
	{
		TestEqual(TEXT("Cancellation notification cannot reactivate"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).Status, EWAbilityRequestStatus::Unavailable);
		Skills->CancelAllAbilities();
	});
	Skills->SetSkillsEnabled(false);
	TestEqual(TEXT("Disable cleans running ability"), Ability->GetState(), EWAbilityState::Idle);
	TestEqual(TEXT("Disabled request rejected"), Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).Status, EWAbilityRequestStatus::Unavailable);
	Skills->SetSkillsEnabled(true);
	Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector);
	Skills->GetOwner()->Destroy();
	TestEqual(TEXT("Owner EndPlay cancels execution"), Ability->GetState(), EWAbilityState::Idle);
	TestNull(TEXT("Owner EndPlay clears slot references"), Skills->GetEquippedAbility(EWAbilitySlot::BasicAttack));
	TestWorld.Advance(0.2f);
	TestEqual(TEXT("EndPlay timer never runs"), Ability->TimerCount, 0);
	Skills->OnAbilityEnded.Remove(EndHandle);

	UWSkillComponent* Reentrant = TestWorld.AddSkills(UWAbilityLifecycleTestProbe::StaticClass());
	Reentrant->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector);
	const auto DestroyHandle = Reentrant->OnAbilityEnded.AddLambda([Reentrant](FName, uint64, bool)
	{
		Reentrant->GetOwner()->Destroy();
	});
	Reentrant->CancelAllAbilities();
	TestNull(TEXT("Owner destruction during cancellation safely clears slots"), Reentrant->GetEquippedAbility(EWAbilitySlot::BasicAttack));
	Reentrant->OnAbilityEnded.Remove(DestroyHandle);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAbilityIsolationTest, "ProjectW.Skills.InstanceIsolationAndDefinitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWAbilityIsolationTest::RunTest(const FString& Parameters)
{
	FWSkillTestWorld TestWorld;
	UWSkillComponent* First = TestWorld.AddSkills(UWAbilityLifecycleTestProbe::StaticClass());
	UWSkillComponent* Second = TestWorld.AddSkills(UWAbilityLifecycleTestProbe::StaticClass());
	auto* A = CastChecked<UWAbilityLifecycleTestProbe>(First->GetEquippedAbility(EWAbilitySlot::BasicAttack));
	auto* B = CastChecked<UWAbilityLifecycleTestProbe>(Second->GetEquippedAbility(EWAbilitySlot::BasicAttack));
	TestTrue(TEXT("Distinct character instances"), A != B && A != GetDefault<UWAbilityLifecycleTestProbe>());
	const auto Execution = First->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector);
	A->Commit(Execution.ExecutionId);
	TestTrue(TEXT("Another character can activate"), Second->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).IsAccepted());
	TestEqual(TEXT("Another character has no cooldown"), Second->GetCooldownRemaining(B->GetAbilityId()), 0.0f);
	TestEqual(TEXT("CDO remains idle"), GetDefault<UWAbilityLifecycleTestProbe>()->GetState(), EWAbilityState::Idle);
	TestEqual(TEXT("CDO has no execution"), GetDefault<UWAbilityLifecycleTestProbe>()->GetActiveExecutionId(), uint64(0));

	AddExpectedError(TEXT("Invalid ability definition for slot"), EAutomationExpectedErrorFlags::Contains, 1);
	UWSkillComponent* Mismatch = TestWorld.AddSkills(nullptr, UWAbilityLifecycleTestProbe::StaticClass());
	TestNull(TEXT("Wrong slot definition rejected"), Mismatch->GetEquippedAbility(EWAbilitySlot::SpecialAttack));
	AddExpectedError(TEXT("Duplicate ability ID"), EAutomationExpectedErrorFlags::Contains, 1);
	UWSkillComponent* Duplicate = TestWorld.AddSkills(UWAbilityLifecycleTestProbe::StaticClass(), UWAbilityDuplicateIdTestProbe::StaticClass());
	TestNotNull(TEXT("Original slot preserved"), Duplicate->GetEquippedAbility(EWAbilitySlot::BasicAttack));
	TestNull(TEXT("Duplicate ID rejected"), Duplicate->GetEquippedAbility(EWAbilitySlot::SpecialAttack));
	return true;
}
#endif
