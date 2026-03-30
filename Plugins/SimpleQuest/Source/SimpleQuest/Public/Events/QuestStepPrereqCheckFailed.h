#pragma once

#include "QuestPrerequisiteCheckFailed.h"

#include "QuestStepPrereqCheckFailed.generated.h"

class UQuest;

USTRUCT(BlueprintType)
struct FQuestStepPrereqCheckFailed : public FQuestPrerequisiteCheckFailed
{
	GENERATED_BODY()
	
	FQuestStepPrereqCheckFailed() = default;

	FQuestStepPrereqCheckFailed(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass, const int32 InQuestStepID)
		: FQuestPrerequisiteCheckFailed(InQuestTag, InQuestClass), QuestStepID(InQuestStepID) {}

	UPROPERTY(BlueprintReadWrite)
	int32 QuestStepID = -1;	
};