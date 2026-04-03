// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Nodes/QuestlineNode_Entry.h"

const FName UQuestlineNode_Entry::PinName_AnyOutcome = TEXT("Any Outcome");
const FName UQuestlineNode_Entry::PinName_Success = TEXT("On Success");
const FName UQuestlineNode_Entry::PinName_Failure = TEXT("On Failure");

void UQuestlineNode_Entry::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, TEXT("QuestSuccess"), PinName_Success);
	CreatePin(EGPD_Output, TEXT("QuestFailure"), PinName_Failure);
	CreatePin(EGPD_Output, TEXT("QuestActivation"), PinName_AnyOutcome);

}


FText UQuestlineNode_Entry::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("SimpleQuestEditor", "EntryNodeTitle", "Start");
}
