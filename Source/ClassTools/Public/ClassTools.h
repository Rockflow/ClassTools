// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FClassToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	TSharedPtr<class RenameDlg> RenameDialog;

	void MakeComboButton();
	void MakeComboList(UToolMenu* ToolMenu);

public:

	void GetAssetPath(TArray<FString>& PathToExploresHeader, TArray<FString>& PathToExploresSource, TArray<FAssetData>& SelectAssetDatas);
	void NotSelectAssetPopup();
	void DeleteButtonClicked();
	void RenameButtonClicked();

	void GetProjectXmlFile(FString& ProjectFileName, FString& ProjectFileFilterName);
	void XmlFindDelete(const char* XmlPath, const char* DeleteName);
	void RemoveFile(const char* Path);
	void XmlFindRename(const char* XmlPath, const char* DeleteName, const char* RenameName);
	void RenameFile(const char* OriginPath, const char* RenamePath);
	void ReBuildAndRestart();
	void ClassDeleteProcess(TArray<FString>& PathToExploresHeader, TArray<FString>& PathToExploresSource, TArray<FAssetData>& SelectAssetDatas);
	void ClassRenameProcess(TArray<FString>& PathToExploresHeader, TArray<FString>& PathToExploresSource, TArray<FAssetData>& SelectAssetDatas,
		TArray<class SRichTextBlock*> NewTextBlockHeader, TArray<class SRichTextBlock*> NewTextBlockSource, TArray<FString>& RenameClass, TArray<bool>& ChangeClick);

	void RenameDlgAllClose();
	
};
