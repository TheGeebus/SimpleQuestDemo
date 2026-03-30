// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Objectives/QuestObjective.h"
#include "KillClassQuestObjective.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLEQUEST_API UKillClassQuestObjective : public UQuestObjective
{
	GENERATED_BODY()

public:
	virtual void TryCompleteObjective_Implementation(UObject* InTargetObject) override;

};
