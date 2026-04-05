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
	UI_COMMAND(NavigateBack, "Back", "Navigate to the previous graph", EUserInterfaceActionType::Button, FInputChord(EKeys::Left, false, false, true, false));
	UI_COMMAND(NavigateForward, "Forward", "Navigate to the next graph", EUserInterfaceActionType::Button, FInputChord(EKeys::Right, false, false, true, false));
}

#undef LOCTEXT_NAMESPACE
