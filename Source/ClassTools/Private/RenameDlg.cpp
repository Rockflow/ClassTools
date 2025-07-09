// Copyright (c) 2017-2018 nextserioss All Rights Reserved.

#include "RenameDlg.h"

#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SHeader.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetData.h"
#include "Misc/FileHelper.h"
#include "UnrealEdMisc.h"

#include "ClassTools.h"

RenameDlg::RenameDlg(FClassToolsModule* Module)
{
	PluginModule = Module;
}

TSharedRef<SDockTab> RenameDlg::MakeWidgetTab(const FSpawnTabArgs& arg)
{
	const FString AssetIndex = arg.GetTabId().ToString();
	FString OldAssetName;
	FString OldStr;
	bool bIsHeader = false;
	FString LeftString;
	FString RightString;
	AssetIndex.Split(TEXT(" "), &LeftString, &RightString);
	if (LeftString.Find(TEXT(".h")) != INDEX_NONE)
	{
		int32 Index = FCString::Atoi(*RightString);
		OldAssetName = FString::Printf(TEXT("%s.h"), *SelectAssetDatas[Index].AssetName.ToString());
		FFileHelper::LoadFileToString(OldStr, *PathToExploresHeader[Index]);
		bIsHeader = true;
	}
	else if (LeftString.Find(TEXT(".cpp")) != INDEX_NONE)
	{
		int32 Index = FCString::Atoi(*RightString);
		OldAssetName = FString::Printf(TEXT("%s.cpp"), *SelectAssetDatas[Index].AssetName.ToString());
		FFileHelper::LoadFileToString(OldStr, *PathToExploresSource[Index]);
		bIsHeader = false;
	}

	TSharedRef<SEditableTextBox> EditableTextBox = SNew(SEditableTextBox).Text(FText::FromString(TEXT(""))).MinDesiredWidth(250);
	TSharedRef<STextBlock> OldHeaderTextBlock = SNew(STextBlock).Text(FText::FromString(OldAssetName));
	TSharedRef<STextBlock> NewHeaderTextBlock = SNew(STextBlock).Text(FText::FromString(OldAssetName));
	TSharedRef<SRichTextBlock> OldTextBlock = SNew(SRichTextBlock).Text(FText::FromString(OldStr));
	TSharedRef<SRichTextBlock> NewTextBlock = SNew(SRichTextBlock).Text(FText::FromString(OldStr));

	if (bIsHeader)
	{
		EditableBoxHeader.Add(&EditableTextBox.Get());
		OldHeaderTextBlockHeader.Add(&OldHeaderTextBlock.Get());
		NewHeaderTextBlockHeader.Add(&NewHeaderTextBlock.Get());
		OldTextBlockHeader.Add(&OldTextBlock.Get());
		NewTextBlockHeader.Add(&NewTextBlock.Get());
	}
	else
	{
		EditableBoxSource.Add(&EditableTextBox.Get());
		OldHeaderTextBlockSource.Add(&OldHeaderTextBlock.Get());
		NewHeaderTextBlockSource.Add(&NewHeaderTextBlock.Get());
		OldTextBlockSource.Add(&OldTextBlock.Get());
		NewTextBlockSource.Add(&NewTextBlock.Get());
	}

	TSharedRef<SDockTab> DockTab = SNew(SDockTab).TabRole(ETabRole::NomadTab)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().HAlign(HAlign_Center).AutoHeight()
				[
					SNew(SHeader).Content()
					[
						OldHeaderTextBlock
					]
				]
				+ SVerticalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						OldTextBlock
					]
				]
				+ SVerticalBox::Slot().HAlign(HAlign_Right).AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).VAlign(VAlign_Center).AutoWidth()
					[
						SNew(STextBlock).Text(FText::FromString("Change Name : "))
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).VAlign(VAlign_Center).AutoWidth()
					[
						EditableTextBox
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).VAlign(VAlign_Center).AutoWidth()
					[
						SNew(SButton).Text(FText::FromString(TEXT("Change")))
						.OnClicked(this, &RenameDlg::OnChangeClick, EAppReturnType::Ok)
					]
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT(" ▶   ")))
			]
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().HAlign(HAlign_Center).AutoHeight()
				[
					SNew(SHeader).Content()
					[
						NewHeaderTextBlock
					]
				]
				+ SVerticalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						NewTextBlock
					]
				]
				+ SVerticalBox::Slot().HAlign(HAlign_Right).AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).AutoWidth()
					[
						SNew(SButton).Text(FText::FromString(TEXT("    Replace    ")))
						.OnClicked(this, &RenameDlg::OnReplaceClick, EAppReturnType::Ok)
					]
				]
			]
		]
	];

	ClassDockTab.Add(&DockTab.Get());
	(&DockTab.Get())->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &RenameDlg::OnTabClosed));
	ChangeClick.Add(false);
	return DockTab;
}

void RenameDlg::ShowDlg(TArray<FString>& PathToHeader, TArray<FString>& PathToSource, TArray<FAssetData>& SelectAssets)
{
	GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,FString("open rename panel"));
	this->PathToExploresHeader = PathToHeader;
	this->PathToExploresSource = PathToSource;
	this->SelectAssetDatas = SelectAssets;
	TArray<FString> RegisterTabIdArray;
	for (int32 i = 0; i < PathToExploresHeader.Num(); i++)
	{
		FString HeaderString = FString::Printf(TEXT("%s.h %d"), *SelectAssetDatas[i].AssetName.ToString(), i);
		FName Header(*(HeaderString));
		RegisterTabIdArray.Add(HeaderString);
		FString SourceString = FString::Printf(TEXT("%s.cpp %d"), *SelectAssetDatas[i].AssetName.ToString(), i);
		FName Source(*(SourceString));
		RegisterTabIdArray.Add(SourceString);
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(Header, FOnSpawnTab::CreateRaw(this, &RenameDlg::MakeWidgetTab));
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(Source, FOnSpawnTab::CreateRaw(this, &RenameDlg::MakeWidgetTab));
	}

	TSharedRef<FTabManager::FStack> TabStack = FTabManager::NewStack();
	for (int32 i = 0; i < PathToExploresHeader.Num(); i++)
	{
		FString HeaderString = FString::Printf(TEXT("%s.h %d"), *SelectAssetDatas[i].AssetName.ToString(), i);
		FName Header(*(HeaderString));
		FString SourceString = FString::Printf(TEXT("%s.cpp %d"), *SelectAssetDatas[i].AssetName.ToString(), i);
		FName Source(*(SourceString));
		TabStack->AddTab(Header, ETabState::OpenedTab);
		TabStack->AddTab(Source, ETabState::OpenedTab);
	}

	TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("TestTab")->AddArea
	(
		FTabManager::NewArea(800, 600)
		->SetWindow(FVector2D(410, 20), false)
		->Split
		(
			TabStack
		)
	);
	TSharedPtr<SWidget> RenameWidget = FGlobalTabmanager::Get()->RestoreFrom(Layout, TSharedPtr<SWindow>());

	for (int32 i = 0; i < RegisterTabIdArray.Num(); i++)
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName(*(RegisterTabIdArray[i])));
	}
}

FReply RenameDlg::OnChangeClick(EAppReturnType::Type ButtonID)
{
	if (ButtonID == EAppReturnType::Ok)
	{
		int32 Foregroundindex = 0;
		for (int32 i = 0; i < ClassDockTab.Num(); i++)
		{
			if (ClassDockTab[i]->IsForeground())
			{
				Foregroundindex = i;
			}
		}

		int32 Index = 0;
		FText ChangeText;
		FString AssetName;
		FString DockId = ClassDockTab[Foregroundindex]->GetLayoutIdentifier().ToString();
		FString LeftString;
		FString RightString;
		DockId.Split(TEXT(" "), &LeftString, &RightString);
		if (LeftString.Find(TEXT(".h")) != INDEX_NONE)
		{
			Index = FCString::Atoi(*RightString);
			ChangeText = EditableBoxHeader[Index]->GetText();
			EditableBoxSource[Index]->SetText(ChangeText);
			AssetName = OldHeaderTextBlockHeader[Index]->GetText().ToString();
			AssetName.RemoveFromEnd(TEXT(".h"));
		}
		else if (LeftString.Find(TEXT(".cpp")) != INDEX_NONE)
		{
			Index = FCString::Atoi(*RightString);
			ChangeText = EditableBoxSource[Index]->GetText();
			EditableBoxHeader[Index]->SetText(ChangeText);
			AssetName = OldHeaderTextBlockSource[Index]->GetText().ToString();
			AssetName.RemoveFromEnd(TEXT(".cpp"));
		}

		FString NewHeaderString = FString::Printf(TEXT("%s.h"), *ChangeText.ToString());
		NewHeaderTextBlockHeader[Index]->SetText(FText::FromString(NewHeaderString));
		FString HeaderChangeString = OldTextBlockHeader[Index]->GetText().ToString().Replace(*AssetName, *ChangeText.ToString());
		NewTextBlockHeader[Index]->SetText(FText::FromString(HeaderChangeString));
		NewTextBlockHeader[Index]->SetHighlightText(ChangeText);

		FString NewSourceString = FString::Printf(TEXT("%s.cpp"), *ChangeText.ToString());
		// NewHeaderTextBlockSource[Index]->SetText(NewSourceString);
		NewHeaderTextBlockSource[Index]->SetText(FText::FromString(NewSourceString));
		FString SourceChangeString = OldTextBlockSource[Index]->GetText().ToString().Replace(*AssetName, *ChangeText.ToString());
		NewTextBlockSource[Index]->SetText(FText::FromString(SourceChangeString));
		NewTextBlockSource[Index]->SetHighlightText(ChangeText);

		ChangeClick[Index] = true;
	}
	return FReply::Handled();
}

FReply RenameDlg::OnReplaceClick(EAppReturnType::Type ButtonID)
{
	if (ButtonID == EAppReturnType::Ok)
	{
		TArray<FString> RenameClass;
		for (int i = 0; i < EditableBoxHeader.Num(); i++)
		{
			RenameClass.Add(EditableBoxHeader[i]->GetText().ToString());
		}
		PluginModule->ClassRenameProcess(PathToExploresHeader, PathToExploresSource, SelectAssetDatas,
			NewTextBlockHeader, NewTextBlockSource, RenameClass, ChangeClick);
	}
	return FReply::Handled();
}

void RenameDlg::OnTabClosed(TSharedRef<SDockTab> Tab)
{
	if (bIsClosed == true)
	{
		return;
	}

	ClassDockTab.Remove(&Tab.Get());
	for (int i = 0; i < ClassDockTab.Num(); i++)
	{
		ClassDockTab[i]->RequestCloseTab();
	}

	if (ClassDockTab.Num() == 0)
	{
		PluginModule->RenameDlgAllClose();
		bIsClosed = true;
	}

}
