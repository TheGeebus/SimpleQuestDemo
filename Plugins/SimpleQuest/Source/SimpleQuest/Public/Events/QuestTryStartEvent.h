// Copyright 2026, Greg Bussell, All Rights Reserved.

#pragma once

#include "Events/QuestEventBase.h"
#include "QuestTryStartEvent.generated.h"

/**
 * Published by a quest giver when the player requests to start a quest.
 * The QuestManagerSubsystem subscribes to this event and calls StartQuest if prerequisites are met.
 * Distinct from FQuestStartedEvent, which is published after the quest has actually started.
 */
USTRUCT(BlueprintType)
struct FTryQuestStartEvent : public FQuestEventBase
{
	GENERATED_BODY()

	FTryQuestStartEvent() = default;

	explicit FTryQuestStartEvent(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass)
		: FQuestEventBase(InQuestTag, InQuestClass) {}
};
