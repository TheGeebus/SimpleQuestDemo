// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Nodes/QuestlineNode_Leaf.h"
#include "Quests/QuestStep.h"

FText UQuestlineNode_Leaf::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!NodeLabel.IsEmpty()) return NodeLabel;
	if (StepClass) return FText::FromString(StepClass->GetName());
	return NSLOCTEXT("SimpleQuestEditor", "LeafNodeDefaultTitle", "Quest Step");
}
