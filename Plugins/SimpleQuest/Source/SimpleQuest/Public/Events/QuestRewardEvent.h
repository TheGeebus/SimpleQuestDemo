#pragma once

#include "Events/QuestEventBase.h"

#include "QuestRewardEvent.generated.h"

class UQuestReward;

USTRUCT(BlueprintType)
struct FQuestRewardEvent : public FQuestEventBase
{
	GENERATED_BODY()
	
	FQuestRewardEvent() = default;

	FQuestRewardEvent(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass, UQuestReward* InRewardObject)
		: FQuestEventBase(InQuestTag, InQuestClass), RewardObject(InRewardObject) {}
	
	UPROPERTY(BlueprintReadWrite)
	UQuestReward* RewardObject = nullptr;
};
