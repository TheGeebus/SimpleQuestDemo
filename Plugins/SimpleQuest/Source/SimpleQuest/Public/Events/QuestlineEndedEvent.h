#pragma once

#include "Events/QuestEventBase.h"

#include "QuestlineEndedEvent.generated.h"

USTRUCT(BlueprintType)
struct FQuestlineEndedEvent : public FQuestEventBase
{
	GENERATED_BODY()

	FQuestlineEndedEvent() = default;

	FQuestlineEndedEvent(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass, const bool bInDidSucceed)
		: FQuestEventBase(InQuestTag, InQuestClass), bDidSucceed(bInDidSucceed) {}

	UPROPERTY(BlueprintReadWrite)
	bool bDidSucceed = false;
};
