#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WMissionDefinition.generated.h"

USTRUCT(BlueprintType)
struct PROJECTW_API FWMissionDefinition : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName DisplayName = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TSoftObjectPtr<UWorld> MissionLevel;

	/** 왕복 검증용 단일 목표. 실제 구조·처치·진압 정의는 후속 작업이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Development")
	FName DevelopmentObjectiveId = TEXT("PrototypeComplete");
};
