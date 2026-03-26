// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Nodes/QuestlineNode_Quest.h"
#include "Quests/Quest.h"

void UQuestlineNode_Quest::AllocateDefaultPins()
{
	// Input pins
	CreatePin(EGPD_Input, TEXT("QuestActivation"), TEXT("Activate"));
	CreatePin(EGPD_Input, TEXT("QuestActivation"), TEXT("Prerequisites"));

	// Output pins
	CreatePin(EGPD_Output, TEXT("QuestSuccess"), TEXT("Success"));
	CreatePin(EGPD_Output, TEXT("QuestFailure"), TEXT("Failure"));
	CreatePin(EGPD_Output, TEXT("QuestActivation"), TEXT("Any Outcome"));
}

FText UQuestlineNode_Quest::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!NodeLabel.IsEmpty())
	{
		return NodeLabel;
	}
	if (QuestClass)
	{
		return FText::FromString(QuestClass->GetName());
	}
	return NSLOCTEXT("SimpleQuestEditor", "QuestNodeDefaultTitle", "Quest");
}

void UQuestlineNode_Quest::AutowireNewNode(UEdGraphPin* FromPin)
{
	if (!FromPin) return;

	const UEdGraphSchema* Schema = GetSchema();

	if (FromPin->Direction == EGPD_Output)
	{
		// Dragged from an output pin — connect to our Activate input
		if (UEdGraphPin* ActivatePin = FindPin(TEXT("Activate")))
		{
			if (Schema->TryCreateConnection(FromPin, ActivatePin))
			{
				FromPin->GetOwningNode()->NodeConnectionListChanged();
			}
		}
	}
	else if (FromPin->Direction == EGPD_Input)
	{
		// Dragged from an input pin — connect our AnyOutcome output to it
		if (UEdGraphPin* AnyOutcomePin = FindPin(TEXT("Any Outcome")))
		{
			if (Schema->TryCreateConnection(AnyOutcomePin, FromPin))
			{
				FromPin->GetOwningNode()->NodeConnectionListChanged();
			}
		}
	}
}

