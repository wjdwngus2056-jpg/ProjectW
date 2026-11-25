// Copyright (C) 2026 TeamW. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WMissionLoopHUD.generated.h"

/** 스냅샷 조회와 복귀 요청만 담당하는 프로토타입 HUD. */
UCLASS()
class PROJECTW_API AWMissionLoopHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	virtual void NotifyHitBoxClick(FName BoxName) override;
protected:
	virtual void BeginPlay() override;
private:
	void RequestReturn();
};
