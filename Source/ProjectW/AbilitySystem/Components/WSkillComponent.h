// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectW/AbilitySystem/Base/WAbilityContext.h"
#include "WSkillComponent.generated.h"

class UWAbility;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FWAbilityEnded, FName, uint64, bool);

/** 고정 장착 슬롯과 캐릭터별 Ability 인스턴스·쿨다운의 소유자입니다. */
UCLASS(ClassGroup = (Ability), meta = (BlueprintSpawnableComponent))
class PROJECTW_API UWSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWSkillComponent();
	FWAbilityRequestResult TryActivateSlot(EWAbilitySlot Slot, const FVector& AimDirection);
	void ReleaseSlot(EWAbilitySlot Slot, uint64 ExecutionId);
	void CancelAllAbilities();
	// 사망/미션 종료 담당 객체가 새 요청을 막고 현재 실행을 정리할 때 사용합니다.
	void SetSkillsEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Ability")
	UWAbility* GetEquippedAbility(EWAbilitySlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	float GetCooldownRemaining(FName AbilityId) const;

	FWAbilityEnded OnAbilityEnded;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	friend class UWAbility;
	void InitializeSlot(EWAbilitySlot Slot, TSubclassOf<UWAbility> AbilityClass);
	EWAbilityRequestStatus CanUseAbility(const UWAbility& Ability) const;
	void StartAbilityCooldown(const UWAbility& Ability, float Duration);

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UWAbility> BasicAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UWAbility> SpecialAbilityClass;

	UPROPERTY(Transient)
	TMap<EWAbilitySlot, TObjectPtr<UWAbility>> EquippedAbilities;

	// 슬롯이 아닌 안정적인 스킬 ID에 기록합니다. 장착 변경과 별개인 캐릭터 수명 상태입니다.
	TMap<FName, double> CooldownEndTimes;
	uint64 LastExecutionId = 0;
	bool bSkillsEnabled = true;
	bool bCancelling = false;
	bool bEndingPlay = false;
};
