// Copyright (C) 2026 TeamW. All Rights Reserved.

#include "WAbilityTestTypes.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UWAbilityLifecycleTestProbe::UWAbilityLifecycleTestProbe()
{
	AbilityId = TEXT("Test.Lifecycle");
	BaseCooldownSeconds = 1.0f;
}

UWAbilityDuplicateIdTestProbe::UWAbilityDuplicateIdTestProbe()
{
	AllowedSlot = EWAbilitySlot::SpecialAttack;
}

#if WITH_DEV_AUTOMATION_TESTS
void UWAbilityLifecycleTestProbe::OnActivate(const FWAbilityContext& Context)
{
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &UWAbilityLifecycleTestProbe::TimerElapsed, 0.1f, false);
}

void UWAbilityLifecycleTestProbe::OnEnd(bool bCancelled)
{
	++EndCount;
	bLastCancelled = bCancelled;
	// 종료 훅의 중복 종료는 공통 경로에서 무시되어야 합니다.
	CancelAbility(GetActiveExecutionId());
}
#endif

AWAbilityDamageTestTarget::AWAbilityDamageTestTarget()
{
	UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	SetRootComponent(Box);
	Box->SetBoxExtent(FVector(20.0f, 40.0f, 40.0f));
	Box->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

float AWAbilityDamageTestTarget::TakeDamage(float Damage, const FDamageEvent& Event, AController* EventInstigator, AActor* Causer)
{
	++HitCount;
	TotalDamage += Damage;
	return Damage;
}
