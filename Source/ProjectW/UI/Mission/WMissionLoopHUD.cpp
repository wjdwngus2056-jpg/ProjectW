// Copyright (C) 2026 TeamW. All Rights Reserved.
#include "WMissionLoopHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "InputMappingContext.h"
#include "Components/InputComponent.h"
#include "ProjectW/Actors/Player/WPlayerCharacter.h"
#include "ProjectW/Actors/Interfaces/WInteractable.h"
#include "ProjectW/Core/Framework/WPlayerController.h"
#include "ProjectW/FireStation/Framework/WFireStationGameMode.h"
#include "ProjectW/Mission/Framework/WMissionGameMode.h"
#include "ProjectW/Mission/Framework/WMissionGameState.h"

void AWMissionLoopHUD::BeginPlay()
{
	Super::BeginPlay();
	// 캐릭터와 입력 수명을 분리하여 초기화 실패 결과에서도 복귀를 요청할 수 있게 한다.
	if (PlayerOwner && PlayerOwner->IsLocalController())
	{
		EnableInput(PlayerOwner);
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ThisClass::RequestReturn);
		InputComponent->BindKey(EKeys::T, IE_Pressed, this, &ThisClass::RequestDevelopmentFinish);
	}
}

void AWMissionLoopHUD::RequestDevelopmentFinish()
{
	// 화면은 조건을 판정하지 않고 별도 테스트 요청을 전달한다. 구조 보고는 이 함수를 호출하지 않는다.
	if (AWMissionGameMode* Mode = GetWorld()->GetAuthGameMode<AWMissionGameMode>()) Mode->TryFinishDevelopmentMission(PlayerOwner);
}

void AWMissionLoopHUD::RequestReturn()
{
	// 테스트용 즉시 복귀 요청만 전달한다. 화면이 성공이나 이동 허용 여부를 결정하지 않는다.
	if (AWMissionGameMode* Mode = GetWorld()->GetAuthGameMode<AWMissionGameMode>()) Mode->TryReturnToStation(PlayerOwner);
}

void AWMissionLoopHUD::NotifyHitBoxClick(FName BoxName)
{
	Super::NotifyHitBoxClick(BoxName);
	if (BoxName == TEXT("ReturnToStation")) RequestReturn();
	if (BoxName == TEXT("DevelopmentFinish")) RequestDevelopmentFinish();
}

void AWMissionLoopHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas || !PlayerOwner) return;
	// 화면 교체 뒤에도 같은 GameState 조회와 GameMode 요청 계약을 사용한다.
	const AWMissionGameState* State = GetWorld()->GetGameState<AWMissionGameState>();
	const AWFireStationGameMode* Station = GetWorld()->GetAuthGameMode<AWFireStationGameMode>();
	float Y = 26.0f;
	auto Line = [this, &Y](const FString& Text, FLinearColor Color = FLinearColor::White)
	{
		DrawText(Text, Color, 30.0f, Y, GEngine->GetMediumFont(), 1.1f);
		Y += 30.0f;
	};
	// 임시 UI 표시를 별도 첫 줄로 두고, 안내·버튼·클릭 영역을 같은 행 간격만큼 함께 내린다.
	DrawRect(FLinearColor(0.01f, 0.02f, 0.03f, 0.86f), 16.0f, 16.0f, Canvas->SizeX - 32.0f, State ? 340.0f : 240.0f);
	Line(TEXT("[테스트용 임시 UI] 시민 구조·목표 보고·왕복 확인"), FLinearColor::Yellow);
	if (State && State->HasResult())
	{
		const FWMissionResult Result = State->GetResult();
		Line(Result.bSucceeded ? TEXT("미션 완료") : TEXT("미션 시작 실패"), Result.bSucceeded ? FLinearColor::Green : FLinearColor::Yellow);
		Line(FString::Printf(TEXT("%s  |  목표 %d / %d"), *Result.MissionId.ToString(), Result.CompletedObjectives, Result.RequiredObjectives));
		Line(Result.Message.ToString());
		Line(State->GetTravelError().ToString(), FLinearColor::Yellow);
		if (State->GetPhase() == EWMissionPhase::Returning)
		{
			Line(TEXT("소방서로 복귀 중..."));
		}
		else
		{
			const FVector2D Position(30.0f, Y + 12.0f);
			DrawRect(FLinearColor(0.08f, 0.28f, 0.4f, 1.0f), Position.X, Position.Y, 380.0f, 48.0f);
			DrawText(TEXT("[테스트 R] 소방서 즉시 복귀 (클릭)"), FLinearColor::White, Position.X + 12, Position.Y + 12, GEngine->GetMediumFont());
			// 새 표시 줄을 반영한 버튼 좌표를 그대로 사용해 보이는 영역과 클릭 영역을 맞춘다.
			AddHitBox(Position, FVector2D(380, 48), TEXT("ReturnToStation"), true);
		}
		return;
	}
	if (State)
	{
		Line(FString::Printf(TEXT("시민 구조: %d / %d  |  %s"), State->GetCompletedObjectives(), State->GetRequiredObjectives(), *State->GetMissionId().ToString()));
		Line(FString::Printf(TEXT("현재 시도: %s"), *State->GetAttemptId().ToString(EGuidFormats::Short)));
		if (State->AreRequiredObjectivesComplete())
		{
			Line(TEXT("시민 구조 완료. 같은 대상을 다시 사용해도 집계되지 않습니다."), FLinearColor::Green);
			Line(TEXT("[테스트] 실제 탈출은 후속 작업입니다. 별도 요청으로 결과를 확인하세요."));
			const AWMissionGameMode* Mode = GetWorld()->GetAuthGameMode<AWMissionGameMode>();
			if (Mode && Mode->CanFinishDevelopmentMission())
			{
				const FVector2D Position(30.0f, Y + 12.0f);
				DrawRect(FLinearColor(0.08f, 0.28f, 0.4f, 1), Position.X, Position.Y, 380, 48);
				DrawText(TEXT("[테스트 T] 결과 확정 (클릭)"), FLinearColor::White, Position.X + 12, Position.Y + 12, GEngine->GetMediumFont());
				AddHitBox(Position, FVector2D(380, 48), TEXT("DevelopmentFinish"), true);
			}
		}
		else Line(TEXT("앞쪽 SOS 시민에게 접근해 상호작용으로 구조하세요."));
	}
	else if (Station)
	{
		Line(TEXT("소방서  |  미션 패널 선택 → 출동 포탈"));
		Line(FString::Printf(TEXT("선택: %s"), *Station->GetSelectedMissionId().ToString()));
		if (Station->HasLastResult())
		{
			const FWMissionResult Result = Station->GetLastResult();
			Line(FString::Printf(TEXT("이전 출동: %s  |  %s  |  목표 %d / %d"), *Result.MissionId.ToString(), Result.bSucceeded ? TEXT("완료") : TEXT("시작 실패"), Result.CompletedObjectives, Result.RequiredObjectives));
		}
		if (!Station->GetTravelError().IsEmpty()) Line(Station->GetTravelError().ToString(), FLinearColor::Yellow);
	}
	if (const AWPlayerCharacter* Character = Cast<AWPlayerCharacter>(PlayerOwner->GetPawn()))
	{
		if (AActor* Target = Character->FindInteractionTarget())
		{
			FString KeyText = TEXT("상호작용");
			if (const AWPlayerController* Controller = Cast<AWPlayerController>(PlayerOwner); Controller && Controller->MainMappingContext)
			{
				for (const FEnhancedActionKeyMapping& Mapping : Controller->MainMappingContext->GetMappings())
				{
					if (Mapping.Action == Controller->InteractionAction) { KeyText = Mapping.Key.GetDisplayName().ToString(); break; }
				}
			}
			Line(FString::Printf(TEXT("[%s] %s"), *KeyText, *Cast<IWInteractable>(Target)->GetInteractionText(const_cast<AWPlayerCharacter*>(Character)).ToString()), FLinearColor::Green);
		}
	}
}
