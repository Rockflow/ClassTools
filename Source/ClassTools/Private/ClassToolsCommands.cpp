// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClassToolsCommands.h"

#define LOCTEXT_NAMESPACE "FClassToolsModule"

void FClassToolsCommands::RegisterCommands()
{
	UI_COMMAND(DeleteAction, "C++Delete", "Execute ClassDelete action", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RenameAction, "C++Rename", "Execute ClassRename action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
