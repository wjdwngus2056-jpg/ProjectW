// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "ProjectW/AbilitySystem/Components/WSkillComponent.h"
#include "UObject/UnrealType.h"

/** 콘텐츠 맵을 수정하지 않고 실제 월드·컴포넌트·충돌을 검증합니다. */
struct FWSkillTestWorld
{
	uint64 SavedFrameCounter = GFrameCounter;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);

	FWSkillTestWorld()
	{
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		World->GetWorldSettings()->NotifyBeginPlay();
		World->BeginPlay();
	}

	~FWSkillTestWorld()
	{
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		GFrameCounter = SavedFrameCounter;
	}

	void Advance(float Seconds)
	{
		while (Seconds > 0.0f)
		{
			const float Step = FMath::Min(Seconds, 1.0f / 60.0f);
			World->Tick(LEVELTICK_All, Step);
			// 엔진의 FTestWorldWrapper와 같이 실제 프레임 진행을 모사합니다.
			++GFrameCounter;
			Seconds -= Step;
		}
	}

	static void SetClass(UObject* Object, FName PropertyName, UClass* Class)
	{
		FindFProperty<FClassProperty>(Object->GetClass(), PropertyName)->SetObjectPropertyValue_InContainer(Object, Class);
	}

	UWSkillComponent* AddSkills(UClass* Basic, UClass* Special = nullptr)
	{
		AActor* Owner = World->SpawnActor<AActor>();
		USphereComponent* Body = NewObject<USphereComponent>(Owner);
		Owner->AddInstanceComponent(Body);
		Owner->SetRootComponent(Body);
		Body->InitSphereRadius(40.0f);
		Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		Body->RegisterComponent();
		Owner->SetActorLocation(FVector(0.0, 0.0, 100.0));

		UWSkillComponent* Skills = NewObject<UWSkillComponent>(Owner);
		Owner->AddInstanceComponent(Skills);
		SetClass(Skills, TEXT("BasicAbilityClass"), Basic);
		SetClass(Skills, TEXT("SpecialAbilityClass"), Special);
		Skills->RegisterComponent();
		return Skills;
	}
};
#endif
