// Copyright 2026, Greg Bussell, All Rights Reserved.


#include "Subsystems/QuestSignalSubsystem.h"

#include "Interfaces/QuestEventListenerInterface.h"


void UQuestSignalSubsystem::Deinitialize()
{
	bIsShuttingDown = true;
	for (auto& Pair : NativeQuestEventChannels)
	{
		Pair.Value.Clear();
	} 
	NativeQuestEventChannels.Empty();
	for (auto& Pair : TaggedEventChannels)
	{
		Pair.Value.Clear();
	}
	TaggedEventChannels.Empty();
	Super::Deinitialize();
}

void UQuestSignalSubsystem::UnsubscribeTypedByTag(const FGameplayTag EventTag, const FDelegateHandle Handle)
{
	if (TMulticastDelegate<void(const FInstancedStruct&)>* Delegate = TaggedEventChannels.Find(EventTag))
	{
		Delegate->Remove(Handle);
	}
}

void UQuestSignalSubsystem::SubscribeToQuestEvent(const TSubclassOf<UObject>& QuestClass, const UScriptStruct* EventClass, UObject* Listener)
{	
	if (!Listener || !Listener->GetClass()->ImplementsInterface(UQuestEventListenerInterface::StaticClass())) return; // Listener invalid
	if (!IsValid(EventClass)) return;
	
	NativeQuestEventChannels.FindOrAdd(MakeKey(QuestClass, EventClass)).AddLambda(
		[Listener](const FInstancedStruct& Event)
		{
			IQuestEventListenerInterface::Execute_OnQuestEventReceived(Listener, Event);
		}
	);
}

void UQuestSignalSubsystem::PublishQuestEvent(const TSubclassOf<UObject>& QuestClass, const FInstancedStruct& Event)
{
	if (!bIsShuttingDown)
	{
		const FSignalEventChannelKey Channel = MakeKey(QuestClass, Event.GetScriptStruct());
		if (const auto* Delegate = NativeQuestEventChannels.Find(Channel))
		{
			Delegate->Broadcast(Event);
		}
	}
}

void UQuestSignalSubsystem::UnsubscribeFromQuestEvent(const TSubclassOf<UObject>& QuestClass, const UScriptStruct* Event, const UObject* Listener)
{
	if (!IsValid(Event) || !IsValid(Listener)) return;
	NativeQuestEventChannels.Find(MakeKey(QuestClass, Event))->RemoveAll(Listener);
}

FSignalEventChannelKey UQuestSignalSubsystem::MakeKey(const UObject* Object, const UScriptStruct* Struct)
{
	return FSignalEventChannelKey(Object, Struct);
}
