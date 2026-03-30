# pragma once

#include "Events/QuestObjectiveTriggered.h"

#include "QuestObjectiveKilled.generated.h"

USTRUCT(BlueprintType)
struct FQuestObjectiveKilled : public FQuestObjectiveTriggered
{
	GENERATED_BODY()

	FQuestObjectiveKilled() = default;

	FQuestObjectiveKilled(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass, AActor* InVictimActor, AActor* InKillerActor)
		: FQuestObjectiveTriggered(InQuestTag, InQuestClass, InVictimActor), KillerActor(InKillerActor) {}

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> KillerActor;
};