// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UWGameplayInputComponent;

/** 플레이어 컨트롤러 */
UCLASS()
class PROJECTW_API AWPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AWPlayerController();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	/** 캐릭터가 마우스 커서 위치를 보도록 보간 회전 */
	void LookCursor();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MainMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
};
