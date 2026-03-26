// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Nodes/QuestNode_Entry.h"

const FName UQuestGraphNode_Entry::PinName_Success(TEXT("Activate: Success"));
const FName UQuestGraphNode_Entry::PinName_Failure(TEXT("Activate: Failure"));
const FName UQuestGraphNode_Entry::PinName_Either(TEXT("Activate: Any Outcome"));

void UQuestGraphNode_Entry::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, TEXT("QuestActivation"), PinName_Success);
	CreatePin(EGPD_Output, TEXT("QuestActivation"), PinName_Failure);
	CreatePin(EGPD_Output, TEXT("QuestActivation"), PinName_Either);
}

FText UQuestGraphNode_Entry::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("SimpleQuestEditor", "EntryNodeTitle", "Quest Start");
}
