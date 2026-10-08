#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WMissionDefinition.generated.h"

USTRUCT(BlueprintType)
struct PROJECTW_API FWMissionDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
	FName DisplayName = NAME_None;
};
