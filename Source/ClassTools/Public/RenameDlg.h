// Copyright (c) 2017-2018 nextserioss All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/SWindow.h"

class RenameDlg : public TSharedFromThis<RenameDlg>
{
public:
	RenameDlg(class FClassToolsModule* Module);
	class FClassToolsModule* PluginModule;

	TArray<class SDockTab*> ClassDockTab;
	TArray<class SEditableTextBox*> EditableBoxHeader;
	TArray<class SEditableTextBox*> EditableBoxSource;

	TArray<class STextBlock*> OldHeaderTextBlockHeader;
	TArray<STextBlock*> OldHeaderTextBlockSource;

	TArray<class STextBlock*> NewHeaderTextBlockHeader;
	TArray<class STextBlock*> NewHeaderTextBlockSource;

	TArray<class SRichTextBlock*> OldTextBlockHeader;
	TArray<class SRichTextBlock*> OldTextBlockSource;

	TArray<class SRichTextBlock*> NewTextBlockHeader;
	TArray<class SRichTextBlock*> NewTextBlockSource;

	TArray<bool> ChangeClick;

	TArray<FString> PathToExploresHeader;
	TArray<FString> PathToExploresSource;
	TArray<FAssetData> SelectAssetDatas;

	bool bIsClosed = false;

	TSharedRef<class SDockTab> MakeWidgetTab(const class FSpawnTabArgs& arg);

	FReply OnChangeClick(EAppReturnType::Type ButtonID);
	FReply OnReplaceClick(EAppReturnType::Type ButtonID);

	void OnTabClosed(TSharedRef<class SDockTab> Tab);

	void ShowDlg(TArray<FString>& PathToHeader, TArray<FString>& PathToSource, TArray<FAssetData>& SelectAssets);
};

