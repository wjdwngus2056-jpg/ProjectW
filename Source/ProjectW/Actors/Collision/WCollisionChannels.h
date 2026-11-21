// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"

/**
 * 프로젝트 Object Channel의 C++ 이름입니다.
 * 이름·번호는 Config/DefaultEngine.ini의 Engine.CollisionProfile과 함께 관리합니다.
 * 저장된 에셋이 번호를 참조하므로 기존 슬롯을 다른 의미로 재사용하지 않습니다.
 */
namespace WCollisionChannels
{
	inline constexpr ECollisionChannel PlayerCharacter = ECC_GameTraceChannel1;
	inline constexpr ECollisionChannel SkillProjectile = ECC_GameTraceChannel2;
}
