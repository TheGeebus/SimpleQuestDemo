// Copyright 2026, Greg Bussell, All Rights Reserved.

#include "Toolkit/QuestlineGraphEditorCommands.h"

#define LOCTEXT_NAMESPACE "QuestlineGraphEditor"

void FQuestlineGraphEditorCommands::RegisterCommands()
{
	UI_COMMAND(
		CompileQuestlineGraph,
		"Compile",
		"Compile the questline graph, writing inter-quest relationships to each Quest CDO",
		EUserInterfaceActionType::Button,
		FInputChord());
}

#undef LOCTEXT_NAMESPACE
