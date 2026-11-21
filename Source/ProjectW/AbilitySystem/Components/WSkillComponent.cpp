// Copyright (C) 2026 TeamW. All Rights Reserved.

#include "WSkillComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "ProjectW/AbilitySystem/Base/WAbility.h"

UWSkillComponent::UWSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWSkillComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSlot(EWAbilitySlot::BasicAttack, BasicAbilityClass);
	InitializeSlot(EWAbilitySlot::SpecialAttack, SpecialAbilityClass);
}

void UWSkillComponent::InitializeSlot(EWAbilitySlot Slot, TSubclassOf<UWAbility> AbilityClass)
{
	if (!AbilityClass)
	{
		return;
	}
	const UWAbility* Definition = AbilityClass.GetDefaultObject();
	if (AbilityClass->HasAnyClassFlags(CLASS_Abstract) || Definition->AbilityId.IsNone() ||
		Definition->AllowedSlot != Slot || !FMath::IsFinite(Definition->BaseCooldownSeconds) ||
		Definition->BaseCooldownSeconds < 0.0f)
	{
		UE_LOG(LogWSkills, Warning, TEXT("Invalid ability definition for slot %d: %s"),
			static_cast<int32>(Slot), *AbilityClass->GetName());
		return;
	}
	for (const auto& Entry : EquippedAbilities)
	{
		if (Entry.Value->GetAbilityId() == Definition->AbilityId)
		{
			UE_LOG(LogWSkills, Warning, TEXT("Duplicate ability ID: %s"), *Definition->AbilityId.ToString());
			return;
		}
	}
	EquippedAbilities.Add(Slot, NewObject<UWAbility>(this, AbilityClass));
}

void UWSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bEndingPlay = true;
	SetSkillsEnabled(false);
	EquippedAbilities.Empty();
	CooldownEndTimes.Empty();
	Super::EndPlay(EndPlayReason);
}

UWAbility* UWSkillComponent::GetEquippedAbility(EWAbilitySlot Slot) const
{
	const TObjectPtr<UWAbility>* Ability = EquippedAbilities.Find(Slot);
	return Ability ? Ability->Get() : nullptr;
}

FWAbilityRequestResult UWSkillComponent::TryActivateSlot(EWAbilitySlot Slot, const FVector& AimDirection)
{
	if (!bSkillsEnabled || bCancelling || bEndingPlay || !HasBegunPlay() ||
		!IsValid(GetOwner()) || GetOwner()->IsActorBeingDestroyed())
	{
		return { EWAbilityRequestStatus::Unavailable, 0 };
	}
	UWAbility* Ability = GetEquippedAbility(Slot);
	if (!IsValid(Ability))
	{
		return { EWAbilityRequestStatus::EmptySlot, 0 };
	}
	if (AimDirection.ContainsNaN())
	{
		return { EWAbilityRequestStatus::InvalidAim, 0 };
	}
	const FVector Direction = AimDirection.GetSafeNormal();
	if (Direction.ContainsNaN() || Direction.IsNearlyZero())
	{
		return { EWAbilityRequestStatus::InvalidAim, 0 };
	}
	// 0은 무효 식별자입니다. 거절된 요청의 ID도 다시 사용하지 않습니다.
	if (++LastExecutionId == 0)
	{
		++LastExecutionId;
	}
	const FWAbilityContext Context{ LastExecutionId, Direction };
	const EWAbilityRequestStatus Status = Ability->TryActivate(Context);
	UE_LOG(LogWSkills, Verbose, TEXT("Slot=%d ability=%s execution=%llu result=%d"),
		static_cast<int32>(Slot), *Ability->GetAbilityId().ToString(), Context.ExecutionId, static_cast<int32>(Status));
	return { Status, Status == EWAbilityRequestStatus::Accepted ? Context.ExecutionId : 0 };
}

void UWSkillComponent::ReleaseSlot(EWAbilitySlot Slot, uint64 ExecutionId)
{
	if (UWAbility* Ability = GetEquippedAbility(Slot))
	{
		Ability->ReleaseAbility(ExecutionId);
	}
}

void UWSkillComponent::CancelAllAbilities()
{
	if (bCancelling)
	{
		return;
	}
	TGuardValue<bool> Guard(bCancelling, true);
	// 종료 알림에서 소유자가 제거되어 슬롯 맵이 비워져도 안전하게 순회합니다.
	TArray<TObjectPtr<UWAbility>> Abilities;
	EquippedAbilities.GenerateValueArray(Abilities);
	for (UWAbility* Ability : Abilities)
	{
		if (IsValid(Ability))
		{
			Ability->CancelAbility(Ability->GetActiveExecutionId());
		}
	}
}

void UWSkillComponent::SetSkillsEnabled(bool bEnabled)
{
	bSkillsEnabled = bEnabled;
	if (!bEnabled)
	{
		CancelAllAbilities();
	}
}

float UWSkillComponent::GetCooldownRemaining(FName AbilityId) const
{
	const UWorld* World = GetWorld();
	const double* EndTime = CooldownEndTimes.Find(AbilityId);
	return World && EndTime ? static_cast<float>(FMath::Max(0.0, *EndTime - World->GetTimeSeconds())) : 0.0f;
}

EWAbilityRequestStatus UWSkillComponent::CanUseAbility(const UWAbility& Ability) const
{
	if (!bSkillsEnabled || bCancelling || bEndingPlay || !HasBegunPlay() || !GetWorld() ||
		!IsValid(GetOwner()) || GetOwner()->IsActorBeingDestroyed() ||
		GetEquippedAbility(Ability.AllowedSlot) != &Ability)
	{
		return EWAbilityRequestStatus::Unavailable;
	}
	if (Ability.AbilityId.IsNone() || !FMath::IsFinite(Ability.BaseCooldownSeconds) || Ability.BaseCooldownSeconds < 0.0f)
	{
		return EWAbilityRequestStatus::InvalidDefinition;
	}
	return GetCooldownRemaining(Ability.AbilityId) > 0.0f ? EWAbilityRequestStatus::Cooldown : EWAbilityRequestStatus::Accepted;
}

void UWSkillComponent::StartAbilityCooldown(const UWAbility& Ability, float Duration)
{
	if (Duration > 0.0f)
	{
		CooldownEndTimes.Add(Ability.AbilityId, GetWorld()->GetTimeSeconds() + Duration);
	}
}
