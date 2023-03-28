// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ClassToolsStyle.h"

class FClassToolsCommands : public TCommands<FClassToolsCommands>
{
public:

	FClassToolsCommands()
		: TCommands<FClassToolsCommands>(TEXT("ClassTools"), NSLOCTEXT("Contexts", "ClassTools", "ClassTools Plugin"), NAME_None, FClassToolsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> DeleteAction;
	TSharedPtr<FUICommandInfo> RenameAction;
};
