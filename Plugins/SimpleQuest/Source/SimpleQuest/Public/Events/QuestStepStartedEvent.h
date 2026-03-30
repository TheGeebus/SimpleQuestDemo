#pragma once

#include "Events/QuestEventBase.h"

#include "QuestStepStartedEvent.generated.h"

USTRUCT(BlueprintType)
struct FQuestStepStartedEvent : public FQuestEventBase
{
	GENERATED_BODY()
	
	FQuestStepStartedEvent() = default;

	FQuestStepStartedEvent(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass, const int32 InStepID)
		: FQuestEventBase(InQuestTag, InQuestClass), StepID(InStepID) {}

	UPROPERTY(BlueprintReadWrite)
	int32 StepID = -1;
};