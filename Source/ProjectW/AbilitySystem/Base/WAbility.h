// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WAbilityContext.h"
#include "UObject/Object.h"
#include "WAbility.generated.h"

class AActor;
class UWSkillComponent;

/** 클래스 기본값은 정의, 컴포넌트가 소유한 인스턴스는 한 캐릭터의 실행 상태입니다. */
UCLASS(Abstract, Blueprintable)
class PROJECTW_API UWAbility : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintPure, Category = "Ability")
	FName GetAbilityId() const { return AbilityId; }

	UFUNCTION(BlueprintPure, Category = "Ability")
	EWAbilityState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Ability")
	FGameplayTagContainer GetAbilityTags() const { return AbilityTags; }

	uint64 GetActiveExecutionId() const { return ActiveContext.ExecutionId; }

protected:
	virtual bool CanActivateCustom(const FWAbilityContext& Context) const { return true; }
	virtual void OnActivate(const FWAbilityContext& Context);
	virtual void OnInputReleased(uint64 ExecutionId) {}
	virtual void OnEnd(bool bCancelled) {}

	AActor* GetAbilityOwner() const;
	bool CommitAbility(uint64 ExecutionId);
	void EndAbility(uint64 ExecutionId);
	void CancelAbility(uint64 ExecutionId);
	bool IsCommittedExecution(uint64 ExecutionId) const;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FName AbilityId;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTagContainer AbilityTags;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	EWAbilitySlot AllowedSlot = EWAbilitySlot::BasicAttack;

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (ClampMin = "0.0"))
	float BaseCooldownSeconds = 0.0f;

private:
	friend class UWSkillComponent;

	EWAbilityRequestStatus TryActivate(const FWAbilityContext& Context);
	void ReleaseAbility(uint64 ExecutionId);
	void FinishAbility(uint64 ExecutionId, bool bCancelled);

	EWAbilityState State = EWAbilityState::Idle;
	FWAbilityContext ActiveContext;
	float ExecutionCooldownSeconds = 0.0f;
};
