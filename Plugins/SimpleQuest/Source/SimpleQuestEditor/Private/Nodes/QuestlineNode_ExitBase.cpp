#include "Nodes/QuestlineNode_ExitBase.h"

void UQuestlineNode_ExitBase::AutowireNewNode(UEdGraphPin* FromPin)
{
	if (!FromPin || FromPin->Direction != EGPD_Output) return;

	const UEdGraphSchema* Schema = GetSchema();
	if (UEdGraphPin* OutcomePin = FindPin(TEXT("Outcome")))
	{
		if (Schema->TryCreateConnection(FromPin, OutcomePin))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}
