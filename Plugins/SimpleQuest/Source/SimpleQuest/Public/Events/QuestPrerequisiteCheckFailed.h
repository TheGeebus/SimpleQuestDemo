#pragma once

#include "QuestEventBase.h"

#include "QuestPrerequisiteCheckFailed.generated.h"

class UQuest;

USTRUCT(BlueprintType)
struct FQuestPrerequisiteCheckFailed : public FQuestEventBase
{
	GENERATED_BODY()

	FQuestPrerequisiteCheckFailed() = default;

	explicit FQuestPrerequisiteCheckFailed(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass)
		: FQuestEventBase(InQuestTag, InQuestClass) {}	
};