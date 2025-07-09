// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClassToolsStyle.h"
#include "ClassTools.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FClassToolsStyle::StyleInstance = nullptr;

void FClassToolsStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FClassToolsStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FClassToolsStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ClassToolsStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FClassToolsStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ClassToolsStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ClassTools")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ClassTools.DeleteAction", new IMAGE_BRUSH(TEXT("ClassDelete_40x"), Icon20x20));
	Style->Set("ClassTools.RenameAction", new IMAGE_BRUSH(TEXT("ClassRename_40x"), Icon20x20));
	Style->Set("ClassTools.OpenComboAction", new IMAGE_BRUSH(TEXT("Icon128"), Icon20x20));
	return Style;
}

void FClassToolsStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FClassToolsStyle::Get()
{
	return *StyleInstance;
}
