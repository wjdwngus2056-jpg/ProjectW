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
};
