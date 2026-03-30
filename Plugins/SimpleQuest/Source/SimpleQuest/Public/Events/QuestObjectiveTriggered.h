#pragma once

#include "Events/QuestEventBase.h"

#include "QuestObjectiveTriggered.generated.h"

USTRUCT(BlueprintType)
struct FQuestObjectiveTriggered : public FQuestEventBase
{
	GENERATED_BODY()

	FQuestObjectiveTriggered() = default;

	FQuestObjectiveTriggered(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass, AActor* InTriggeredActor)
		: FQuestEventBase(InQuestTag, InQuestClass), TriggeredActor(InTriggeredActor) {}
		
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> TriggeredActor;
};