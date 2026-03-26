// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Nodes/QuestlineNode_Entry.h"

void UQuestlineNode_Entry::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, TEXT("QuestActivation"), TEXT("Start"));
}


FText UQuestlineNode_Entry::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("SimpleQuestEditor", "EntryNodeTitle", "Questline Start");
}
