// Copyright (C) 2026 TeamW. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WMissionLoopHUD.generated.h"

/**
 * 목표·결과·즉시 복귀의 왕복 동작을 확인하는 테스트용 임시 UI.
 * 구조 상태는 도메인에서 조회하고 테스트용 결과 확정·복귀를 요청한다. 정식 HUD는 표시·입력을 교체한다.
 */
UCLASS()
class PROJECTW_API AWMissionLoopHUD : public AHUD
{
	GENERATED_BODY()
public:
	/** 현재 상태를 매 프레임 조회한다. 진행 수량과 결과는 HUD에 보관하지 않는다. */
	virtual void DrawHUD() override;
	/** 임시 완료·복귀 영역 클릭을 각 도메인 요청 경로로 전달한다. */
	virtual void NotifyHitBoxClick(FName BoxName) override;
protected:
	/** 결과로 캐릭터 입력이 막혀도 로컬 HUD의 복귀 입력은 유지한다. */
	virtual void BeginPlay() override;
private:
	/** 구조 상호작용과 별개인 테스트용 결과 확정 요청이다. */
	void RequestDevelopmentFinish();
	/** 결과·요청자·중복 판정과 실제 이동은 GameMode에 맡긴다. */
	void RequestReturn();
};
