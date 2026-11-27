// Copyright (C) 2026 TeamW. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProjectW/Mission/Data/WMissionRuntime.h"
#include "WMissionResultTransfer.generated.h"

/** 확정 결과 한 건의 맵 간 전달함. 진행 중 미션 상태는 보유하지 않는다. */
UCLASS()
class PROJECTW_API UWMissionResultTransfer : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	void Store(const FWMissionResult& Result) { Pending = Result; bPending = true; }
	bool Consume(FWMissionResult& OutResult)
	{
		if (!bPending) return false;
		OutResult = Pending;
		Clear();
		return true;
	}
	void Clear() { Pending = FWMissionResult(); bPending = false; }
private:
	UPROPERTY(Transient) FWMissionResult Pending;
	bool bPending = false;
};
