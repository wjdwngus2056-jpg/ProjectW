// Copyright (C) 2026 TeamW. All Rights Reserved.

#include "WAbility.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "ProjectW/AbilitySystem/Components/WSkillComponent.h"

UWorld* UWAbility::GetWorld() const
{
	const UWSkillComponent* Skills = Cast<UWSkillComponent>(GetOuter());
	return IsValid(Skills) ? Skills->GetWorld() : nullptr;
}

EWAbilityRequestStatus UWAbility::TryActivate(const FWAbilityContext& Context)
{
	if (State != EWAbilityState::Idle)
	{
		return EWAbilityRequestStatus::AlreadyActive;
	}
	const UWSkillComponent* Skills = Cast<UWSkillComponent>(GetOuter());
	if (!IsValid(Skills) || Context.ExecutionId == 0)
	{
		return EWAbilityRequestStatus::Unavailable;
	}
	const EWAbilityRequestStatus Status = Skills->CanUseAbility(*this);
	if (Status != EWAbilityRequestStatus::Accepted)
	{
		return Status;
	}
	if (!CanActivateCustom(Context))
	{
		return EWAbilityRequestStatus::CustomConditionFailed;
	}
	ActiveContext = Context;
	ExecutionCooldownSeconds = BaseCooldownSeconds;
	State = EWAbilityState::Preparing;
	UE_LOG(LogWSkills, Verbose, TEXT("Activate %s execution=%llu"), *AbilityId.ToString(), Context.ExecutionId);
	OnActivate(Context);
	return EWAbilityRequestStatus::Accepted;
}

void UWAbility::ReleaseAbility(uint64 ExecutionId)
{
	if (ExecutionId != 0 && ExecutionId == ActiveContext.ExecutionId &&
		(State == EWAbilityState::Preparing || State == EWAbilityState::Committed))
	{
		OnInputReleased(ExecutionId);
	}
}

void UWAbility::CancelAbility(uint64 ExecutionId)
{
	FinishAbility(ExecutionId, true);
}

void UWAbility::OnActivate(const FWAbilityContext& Context)
{
	CancelAbility(Context.ExecutionId);
}

AActor* UWAbility::GetAbilityOwner() const
{
	const UWSkillComponent* Skills = Cast<UWSkillComponent>(GetOuter());
	return IsValid(Skills) ? Skills->GetOwner() : nullptr;
}

bool UWAbility::IsCommittedExecution(uint64 ExecutionId) const
{
	return ExecutionId != 0 && ExecutionId == ActiveContext.ExecutionId && State == EWAbilityState::Committed;
}

bool UWAbility::CommitAbility(uint64 ExecutionId)
{
	// 같은 실행의 재커밋은 성공 상태를 돌려주되 쿨다운을 다시 시작하지 않습니다.
	if (IsCommittedExecution(ExecutionId))
	{
		return true;
	}
	if (ExecutionId == 0 || ExecutionId != ActiveContext.ExecutionId || State != EWAbilityState::Preparing)
	{
		return false;
	}
	UWSkillComponent* Skills = Cast<UWSkillComponent>(GetOuter());
	if (!IsValid(Skills) || Skills->CanUseAbility(*this) != EWAbilityRequestStatus::Accepted ||
		!CanActivateCustom(ActiveContext))
	{
		CancelAbility(ExecutionId);
		return false;
	}
	Skills->StartAbilityCooldown(*this, ExecutionCooldownSeconds);
	State = EWAbilityState::Committed;
	UE_LOG(LogWSkills, Verbose, TEXT("Commit %s execution=%llu"), *AbilityId.ToString(), ExecutionId);
	return true;
}

void UWAbility::EndAbility(uint64 ExecutionId)
{
	FinishAbility(ExecutionId, false);
}

void UWAbility::FinishAbility(uint64 ExecutionId, bool bCancelled)
{
	if (ExecutionId == 0 || ExecutionId != ActiveContext.ExecutionId ||
		(State != EWAbilityState::Preparing && State != EWAbilityState::Committed))
	{
		return;
	}
	// 종료 훅의 중복 종료/재진입을 먼저 차단합니다. 객체에 바인딩한 타이머도 함께 정리합니다.
	State = EWAbilityState::Ending;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	OnEnd(bCancelled);
	ActiveContext = FWAbilityContext();
	ExecutionCooldownSeconds = 0.0f;
	State = EWAbilityState::Idle;
	UE_LOG(LogWSkills, Verbose, TEXT("End %s execution=%llu cancelled=%d"),
		*AbilityId.ToString(), ExecutionId, bCancelled);
	if (UWSkillComponent* Skills = Cast<UWSkillComponent>(GetOuter()))
	{
		Skills->OnAbilityEnded.Broadcast(AbilityId, ExecutionId, bCancelled);
	}
}
