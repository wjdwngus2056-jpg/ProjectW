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
	if (PlayerOwner && PlayerOwner->IsLocalController())
	{
		EnableInput(PlayerOwner);
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ThisClass::RequestReturn);
	}
}

void AWMissionLoopHUD::RequestReturn()
{
	if (AWMissionGameMode* Mode = GetWorld()->GetAuthGameMode<AWMissionGameMode>()) Mode->TryReturnToStation(PlayerOwner);
}

void AWMissionLoopHUD::NotifyHitBoxClick(FName BoxName)
{
	Super::NotifyHitBoxClick(BoxName);
	if (BoxName == TEXT("ReturnToStation")) RequestReturn();
}

void AWMissionLoopHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas || !PlayerOwner) return;
	const AWMissionGameState* State = GetWorld()->GetGameState<AWMissionGameState>();
	const AWFireStationGameMode* Station = GetWorld()->GetAuthGameMode<AWFireStationGameMode>();
	float Y = 26.0f;
	auto Line = [this, &Y](const FString& Text, FLinearColor Color = FLinearColor::White)
	{
		DrawText(Text, Color, 30.0f, Y, GEngine->GetMediumFont(), 1.1f);
		Y += 30.0f;
	};
	DrawRect(FLinearColor(0.01f, 0.02f, 0.03f, 0.86f), 16.0f, 16.0f, Canvas->SizeX - 32.0f, State && State->HasResult() ? 290.0f : 180.0f);
	if (State && State->HasResult())
	{
		const FWMissionResult Result = State->GetResult();
		Line(Result.bSucceeded ? TEXT("미션 완료") : TEXT("미션 시작 실패"), Result.bSucceeded ? FLinearColor::Green : FLinearColor::Yellow);
		Line(FString::Printf(TEXT("%s  |  목표 %d / 1"), *Result.MissionId.ToString(), Result.CompletedObjectives));
		Line(Result.Message.ToString());
		Line(State->GetTravelError().ToString(), FLinearColor::Yellow);
		if (State->GetPhase() == EWMissionPhase::Returning)
		{
			Line(TEXT("소방서로 복귀 중..."));
		}
		else
		{
			const FVector2D Position(30.0f, Y + 12.0f);
			DrawRect(FLinearColor(0.08f, 0.28f, 0.4f, 1.0f), Position.X, Position.Y, 290.0f, 48.0f);
			DrawText(TEXT("[R] 소방서로 복귀   (클릭)"), FLinearColor::White, Position.X + 12, Position.Y + 12, GEngine->GetMediumFont());
			AddHitBox(Position, FVector2D(290, 48), TEXT("ReturnToStation"), true);
		}
		return;
	}
	if (State)
	{
		Line(FString::Printf(TEXT("개발용 미션: %s  |  목표 %d / 1"), *State->GetMissionId().ToString(), State->GetCompletedObjectives()));
		Line(TEXT("앞쪽 큐브에 접근해 상호작용하면 결과를 확인합니다."));
	}
	else if (Station)
	{
		Line(TEXT("소방서  |  미션 패널 선택 → 출동 포탈"));
		Line(FString::Printf(TEXT("선택: %s"), *Station->GetSelectedMissionId().ToString()));
		if (Station->HasLastResult())
		{
			const FWMissionResult Result = Station->GetLastResult();
			Line(FString::Printf(TEXT("이전 출동: %s  |  %s  |  목표 %d / 1"), *Result.MissionId.ToString(), Result.bSucceeded ? TEXT("완료") : TEXT("시작 실패"), Result.CompletedObjectives));
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
