// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Nodes/QuestlineNode_Quest.h"
#include "Quests/Quest.h"

FText UQuestlineNode_Quest::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!NodeLabel.IsEmpty()) return NodeLabel;
	if (QuestClass) return FText::FromString(QuestClass->GetName());
	return NSLOCTEXT("SimpleQuestEditor", "QuestNodeDefaultTitle", "Quest");
}


