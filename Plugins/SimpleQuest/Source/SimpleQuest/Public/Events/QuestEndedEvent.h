#pragma once

#include "Events/QuestEventBase.h"

#include "QuestEndedEvent.generated.h"

USTRUCT(BlueprintType)
struct FQuestEndedEvent : public FQuestEventBase
{	
	GENERATED_BODY()

	FQuestEndedEvent() = default;

	FQuestEndedEvent(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass, const bool bInDidSucceed)
		: FQuestEventBase(InQuestTag, InQuestClass), bDidSucceed(bInDidSucceed) {}

	UPROPERTY(BlueprintReadWrite)
	bool bDidSucceed = false;
};