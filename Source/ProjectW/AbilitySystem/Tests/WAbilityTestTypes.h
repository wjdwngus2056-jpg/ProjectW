// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "ProjectW/AbilitySystem/Base/WAbility.h"
#include "GameFramework/Actor.h"
#include "WAbilityTestTypes.generated.h"

/** 공통 생명주기를 검사하는 준비 단계 스킬입니다. 콘텐츠에서 사용하지 않습니다. */
UCLASS(NotBlueprintable, Transient)
class UWAbilityLifecycleTestProbe : public UWAbility
{
	GENERATED_BODY()

public:
	UWAbilityLifecycleTestProbe();

#if WITH_DEV_AUTOMATION_TESTS
	bool Commit(uint64 Id) { return CommitAbility(Id); }
	void Finish(uint64 Id) { EndAbility(Id); }
	void Cancel(uint64 Id) { CancelAbility(Id); }
	bool bCondition = true;
	int32 EndCount = 0;
	int32 TimerCount = 0;
	bool bLastCancelled = false;

protected:
	virtual bool CanActivateCustom(const FWAbilityContext& Context) const override { return bCondition; }
	virtual void OnActivate(const FWAbilityContext& Context) override;
	virtual void OnInputReleased(uint64 Id) override { EndAbility(Id); }
	virtual void OnEnd(bool bCancelled) override;

private:
	void TimerElapsed() { ++TimerCount; }
#endif
};

/** 중복 ID가 서로 다른 슬롯에서도 거부되는지 검사합니다. */
UCLASS(NotBlueprintable, Transient)
class UWAbilityDuplicateIdTestProbe : public UWAbilityLifecycleTestProbe
{
	GENERATED_BODY()

public:
	UWAbilityDuplicateIdTestProbe();
};

/** 엔진의 실제 PointDamage와 충돌 횟수를 관찰하는 테스트 대상입니다. */
UCLASS(NotBlueprintable, Transient)
class AWAbilityDamageTestTarget : public AActor
{
	GENERATED_BODY()

public:
	AWAbilityDamageTestTarget();
	virtual float TakeDamage(float Damage, const FDamageEvent& Event, AController* EventInstigator, AActor* Causer) override;
	int32 HitCount = 0;
	float TotalDamage = 0.0f;
};
