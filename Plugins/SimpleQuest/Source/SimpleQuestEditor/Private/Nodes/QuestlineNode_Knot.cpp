// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Nodes/QuestlineNode_Knot.h"

// QuestlineNode_Knot.cpp
void UQuestlineNode_Knot::AllocateDefaultPins()
{
	CreatePin(EGPD_Input,  TEXT("QuestActivation"), TEXT("KnotIn"));
	CreatePin(EGPD_Output, TEXT("QuestActivation"), TEXT("KnotOut"));
}

void UQuestlineNode_Knot::AutowireNewNode(UEdGraphPin* FromPin)
{
	if (!FromPin) return;

	const UEdGraphSchema* Schema = GetSchema();

	// Inherit the category from the source pin so wire color flows through
	UEdGraphPin* KnotIn  = FindPin(TEXT("KnotIn"));
	UEdGraphPin* KnotOut = FindPin(TEXT("KnotOut"));

	if (KnotIn)  KnotIn->PinType  = FromPin->PinType;
	if (KnotOut) KnotOut->PinType = FromPin->PinType;

	// Make connections
	if (FromPin->Direction == EGPD_Output)
	{
		if (Schema->TryCreateConnection(FromPin, KnotIn))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
	else if (FromPin->Direction == EGPD_Input)
	{
		if (Schema->TryCreateConnection(KnotOut, FromPin))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}
