#pragma once

#include "Events/QuestEventBase.h"

#include "QuestRegistrationEvent.generated.h"

USTRUCT(BlueprintType)
struct FQuestRegistrationEvent : public FQuestEventBase
{
	GENERATED_BODY()

	FQuestRegistrationEvent() = default;
	
	FQuestRegistrationEvent(const FGameplayTag InQuestTag, const TSubclassOf<UQuest>& InQuestClass, AActor* InOwningActor)
		: FQuestEventBase(InQuestTag, InQuestClass), OwningActor(InOwningActor) {}

	UPROPERTY()
	TObjectPtr<AActor> OwningActor;
};
