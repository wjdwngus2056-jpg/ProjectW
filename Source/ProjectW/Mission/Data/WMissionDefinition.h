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

	/** 각 ID에 해당하는 월드 대상이 정확히 하나 필요하다. 실행 중 완료 상태와 분리한 정의다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Objectives")
	TArray<FName> RequiredObjectiveIds = { TEXT("RescueCitizen") };
};
