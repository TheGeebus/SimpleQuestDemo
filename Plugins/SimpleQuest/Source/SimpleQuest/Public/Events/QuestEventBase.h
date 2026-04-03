#pragma once

#include "Signals/SignalEventBase.h"
#include "QuestEventBase.generated.h"

class UQuest;

USTRUCT(BlueprintType)
struct FQuestEventBase : public FSignalEventBase
{
	GENERATED_BODY()

	FQuestEventBase() = default;

	FQuestEventBase(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass)
		: FSignalEventBase(InQuestTag.GetTagName(), InQuestTag), QuestClass(InQuestClass)
	{}

	FGameplayTag GetQuestTag() const { return EventTags.IsEmpty() ? FGameplayTag() : EventTags.GetByIndex(0); }
	
	UPROPERTY()
	TSubclassOf<UQuest> QuestClass;
};