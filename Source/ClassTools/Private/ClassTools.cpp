// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClassTools.h"
#include "ClassToolsStyle.h"
#include "ClassToolsCommands.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "SourceCodeNavigation.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Misc/FileHelper.h"
#include "Widgets/Text/SRichTextBlock.h"

#include "tinyxml2.h"
#include "RenameDlg.h"

static const FName ClassToolsTabName("ClassTools");

#define LOCTEXT_NAMESPACE "FClassToolsModule"

void FClassToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FClassToolsStyle::Initialize();
	FClassToolsStyle::ReloadTextures();

	FClassToolsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FClassToolsCommands::Get().DeleteAction,
		FExecuteAction::CreateRaw(this, &FClassToolsModule::DeleteButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FClassToolsCommands::Get().RenameAction,
		FExecuteAction::CreateRaw(this, &FClassToolsModule::RenameButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FClassToolsModule::RegisterMenus));
}

void FClassToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FClassToolsStyle::Shutdown();

	FClassToolsCommands::Unregister();
}

void FClassToolsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	// {
	// 	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	// 	{
	// 		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
	// 		Section.AddMenuEntryWithCommandList(FClassToolsCommands::Get().PluginAction, PluginCommands);
	// 	}
	// }

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& DelEntry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FClassToolsCommands::Get().DeleteAction));
				DelEntry.SetCommandList(PluginCommands);

				FToolMenuEntry& RenEntry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FClassToolsCommands::Get().RenameAction));
				RenEntry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FClassToolsModule::GetAssetPath(TArray<FString>& PathToExploresHeader, TArray<FString>& PathToExploresSource, TArray<FAssetData>& SelectAssetDatas)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	for (int32 AssetIdx = 0; AssetIdx < SelectedAssets.Num(); ++AssetIdx)
	{
		UObject* Asset = SelectedAssets[AssetIdx].GetAsset();
		if (Asset)
		{
			FAssetData AssetData(Asset);
			if (AssetData.AssetClass.Compare(TEXT("Class")) == 0)
			{
				SelectAssetDatas.Add(AssetData);
				const FString PackageName = AssetData.PackageName.ToString();
				static const TCHAR* ScriptString = TEXT("/Script/");

				if (PackageName.StartsWith(ScriptString))
				{
					const FString ModuleName = PackageName.RightChop(FCString::Strlen(ScriptString));

					FString ModulePath;
					if (FSourceCodeNavigation::FindModulePath(ModuleName, ModulePath))
					{
						FString RelativePath;
						if (AssetData.GetTagValue("ModuleRelativePath", RelativePath))
						{
							PathToExploresHeader.Add(FPaths::ConvertRelativePathToFull(ModulePath / (*RelativePath)));
							RelativePath = RelativePath.Replace(TEXT(".h"), TEXT(".cpp"));
							FString SourceString = FPaths::ConvertRelativePathToFull(ModulePath / (*RelativePath));
							FString PrivateSourceString = SourceString.Replace(TEXT("Public"), TEXT("Private"));
							if (FPaths::FileExists(PrivateSourceString))
							{
								PathToExploresSource.Add(PrivateSourceString);
							}
							else
							{
								PathToExploresSource.Add(SourceString);
							}
						}
					}
				}
			}
		}
	}
}

void FClassToolsModule::NotSelectAssetPopup()
{
	FText DialogText = FText::FromString(TEXT("You have not selected a class file."));
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FClassToolsModule::DeleteButtonClicked()
{
	TArray<FString> PathToExploresHeader;
	TArray<FString> PathToExploresSource;
	TArray<FAssetData> SelectAssetDatas;
	GetAssetPath(PathToExploresHeader, PathToExploresSource, SelectAssetDatas);

	if (SelectAssetDatas.Num() == 0)
	{
		NotSelectAssetPopup();
	}
	else
	{
		FString DeleteFiles;
		for (int i = 0; i < SelectAssetDatas.Num(); i++)
		{
			DeleteFiles.Append(*(SelectAssetDatas[i].AssetName.ToString() + TEXT(", ")));
		}
		FString DeleteString = FString::Printf(TEXT("Delete class file : %s"), *DeleteFiles);
		EAppReturnType::Type ReturnType = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(DeleteString));
		if (ReturnType == EAppReturnType::Yes)
		{
			ClassDeleteProcess(PathToExploresHeader, PathToExploresSource, SelectAssetDatas);
		}
	}
}

void FClassToolsModule::RenameButtonClicked()
{
	TArray<FString> PathToExploresHeader;
	TArray<FString> PathToExploresSource;
	TArray<FAssetData> SelectAssetDatas;
	GetAssetPath(PathToExploresHeader, PathToExploresSource, SelectAssetDatas);

	if (SelectAssetDatas.Num() == 0)
	{
		NotSelectAssetPopup();
	}
	else
	{
		if (RenameDialog.IsValid() == true)
		{
			FText DialogText = FText::FromString(TEXT("The Rename pop-up is already open."));
			EAppReturnType::Type ReturnType = FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		}
		else
		{
			RenameDialog = MakeShareable(new RenameDlg(this));
			RenameDialog.Get()->ShowDlg(PathToExploresHeader, PathToExploresSource, SelectAssetDatas);
		}
	}
}

void FClassToolsModule::GetProjectXmlFile(FString& ProjectFileName, FString& ProjectFileFilterName)
{
	ProjectFileName = FPaths::ProjectDir().Append("Intermediate/ProjectFiles/");
	ProjectFileName.Append(FApp::GetProjectName());
	ProjectFileName.Append(".vcxproj");

	ProjectFileFilterName = FPaths::ProjectDir().Append("Intermediate/ProjectFiles/");
	ProjectFileFilterName.Append(FApp::GetProjectName());
	ProjectFileFilterName.Append(".vcxproj.filters");
}

void FClassToolsModule::XmlFindDelete(const char* XmlPath, const char* DeleteName)
{
	tinyxml2::XMLDocument Doc;
	Doc.LoadFile(XmlPath);

	TArray<tinyxml2::XMLElement*> DeleteElements;

	tinyxml2::XMLElement* Element = Doc.FirstChildElement("Project");
	for (tinyxml2::XMLElement* ItemGroup = Element->FirstChildElement("ItemGroup"); ItemGroup != NULL; ItemGroup = ItemGroup->NextSiblingElement())
	{
		for (tinyxml2::XMLElement* ChildItem = ItemGroup->FirstChildElement(); ChildItem != NULL; ChildItem = ChildItem->NextSiblingElement())
		{
			if (strcmp(ChildItem->Name(), "ClCompile") == 0)
			{
				const char* Include = ChildItem->Attribute("Include");
				const char* Findstr;
				Findstr = strstr(Include, DeleteName);
				if (Findstr != nullptr)
				{
					UE_LOG(LogClass, Display, TEXT("Delete : %s"), ANSI_TO_TCHAR(DeleteName));
					DeleteElements.Add(ChildItem);
				}
			}
			else if (strcmp(ChildItem->Name(), "ClInclude") == 0)
			{
				const char* Include = ChildItem->Attribute("Include");
				const char* Findstr;
				Findstr = strstr(Include, DeleteName);
				if (Findstr != nullptr)
				{
					UE_LOG(LogClass, Display, TEXT("Delete : %s"), ANSI_TO_TCHAR(DeleteName));
					DeleteElements.Add(ChildItem);
				}
			}
		}
	}
	for (int i = 0; i < DeleteElements.Num(); i++)
	{
		Doc.DeleteNode(DeleteElements[i]);
	}

	tinyxml2::XMLError XmlError = Doc.SaveFile(XmlPath, false);
	if (XmlError != tinyxml2::XML_SUCCESS)
	{
		UE_LOG(LogClass, Error, TEXT("Xml SaveFile Error"));
	}
}

void FClassToolsModule::RemoveFile(const char* Path)
{
	if (remove(Path) != 0)
	{
		UE_LOG(LogClass, Error, TEXT("File Delete Error"));
	}
	else
	{
		UE_LOG(LogClass, Display, TEXT("Delete File : %s"), ANSI_TO_TCHAR(Path));
	}
}

void FClassToolsModule::XmlFindRename(const char * XmlPath, const char* DeleteName, const char* RenameName)
{
	tinyxml2::XMLDocument Doc;
	Doc.LoadFile(XmlPath);

	TArray<tinyxml2::XMLElement*> RenameElements;

	tinyxml2::XMLElement* Element = Doc.FirstChildElement("Project");
	for (tinyxml2::XMLElement* ItemGroup = Element->FirstChildElement("ItemGroup"); ItemGroup != NULL; ItemGroup = ItemGroup->NextSiblingElement())
	{
		for (tinyxml2::XMLElement* ChildItem = ItemGroup->FirstChildElement(); ChildItem != NULL; ChildItem = ChildItem->NextSiblingElement())
		{
			if (strcmp(ChildItem->Name(), "ClCompile") == 0)
			{
				const char* Include = ChildItem->Attribute("Include");
				const char* Findstr;
				Findstr = strstr(Include, DeleteName);
				if (Findstr != nullptr)
				{
					UE_LOG(LogClass, Display, TEXT("Delete : %s"), ANSI_TO_TCHAR(DeleteName));
					RenameElements.Add(ChildItem);
				}
			}
			else if (strcmp(ChildItem->Name(), "ClInclude") == 0)
			{
				const char* Include = ChildItem->Attribute("Include");
				const char* Findstr;
				Findstr = strstr(Include, DeleteName);
				if (Findstr != nullptr)
				{
					UE_LOG(LogClass, Display, TEXT("Delete : %s"), ANSI_TO_TCHAR(DeleteName));
					RenameElements.Add(ChildItem);
				}
			}
		}
	}
	for (int i = 0; i < RenameElements.Num(); i++)
	{
		const char* Include = RenameElements[i]->Attribute("Include");
		FString strInclude(Include);
		strInclude = strInclude.Replace(ANSI_TO_TCHAR(DeleteName), ANSI_TO_TCHAR(RenameName));
		RenameElements[i]->SetAttribute("Include", TCHAR_TO_ANSI(*strInclude));
	}

	tinyxml2::XMLError XmlError = Doc.SaveFile(XmlPath, false);
	if (XmlError != tinyxml2::XML_SUCCESS)
	{
		UE_LOG(LogClass, Error, TEXT("Xml SaveFile Error"));
	}
}

void FClassToolsModule::RenameFile(const char* OriginPath, const char* RenamePath)
{
	if (rename(OriginPath, RenamePath) != 0)
	{
		UE_LOG(LogClass, Error, TEXT("File Rename Error"));
	}
	else
	{
		UE_LOG(LogClass, Display, TEXT("Rename File : %s"), ANSI_TO_TCHAR(OriginPath));
	}
}

void FClassToolsModule::ReBuildAndRestart()
{
	FString BuildFileName = FPaths::ProjectDir().Append("Binaries/Win64/");
	BuildFileName.Append("UE4Editor.modules");
	RemoveFile(TCHAR_TO_ANSI(*BuildFileName));

	FUnrealEdMisc::Get().AllowSavingLayoutOnClose(true);
	FUnrealEdMisc::Get().RestartEditor(false);
}

void FClassToolsModule::ClassDeleteProcess(TArray<FString>& PathToExploresHeader, TArray<FString>& PathToExploresSource, TArray<FAssetData>& SelectAssetDatas)
{	
	FString ProjectFileName;
	FString ProjectFileFilterName;
	GetProjectXmlFile(ProjectFileName, ProjectFileFilterName);

	for (int i = 0; i < SelectAssetDatas.Num(); i++)
	{
		FString HeaderString = FString::Printf(TEXT("%s.h"), *SelectAssetDatas[i].AssetName.ToString());
		FString SourceString = FString::Printf(TEXT("%s.cpp"), *SelectAssetDatas[i].AssetName.ToString());
		XmlFindDelete(TCHAR_TO_ANSI(*ProjectFileName), TCHAR_TO_ANSI(*HeaderString));
		XmlFindDelete(TCHAR_TO_ANSI(*ProjectFileName), TCHAR_TO_ANSI(*SourceString));
		XmlFindDelete(TCHAR_TO_ANSI(*ProjectFileFilterName), TCHAR_TO_ANSI(*HeaderString));
		XmlFindDelete(TCHAR_TO_ANSI(*ProjectFileFilterName), TCHAR_TO_ANSI(*SourceString));
	}

	//Class File Delete
	for (int i = 0; i < PathToExploresHeader.Num(); i++)
	{
		RemoveFile(TCHAR_TO_ANSI(*(PathToExploresHeader[i])));
		RemoveFile(TCHAR_TO_ANSI(*(PathToExploresSource[i])));
	}

	ReBuildAndRestart();
}

void FClassToolsModule::ClassRenameProcess(TArray<FString>& PathToExploresHeader, TArray<FString>& PathToExploresSource, TArray<FAssetData>& SelectAssetDatas,
	TArray<SRichTextBlock*> NewTextBlockHeader, TArray<SRichTextBlock*> NewTextBlockSource, TArray<FString>& RenameClass, TArray<bool>& ChangeClick)
{
	FString ProjectFileName;
	FString ProjectFileFilterName;
	GetProjectXmlFile(ProjectFileName, ProjectFileFilterName);

	for (int i = 0; i < SelectAssetDatas.Num(); i++)
	{
		if (ChangeClick[i] == true)
		{
			FString OldHeaderString = FString::Printf(TEXT("%s.h"), *SelectAssetDatas[i].AssetName.ToString());
			FString OldSourceString = FString::Printf(TEXT("%s.cpp"), *SelectAssetDatas[i].AssetName.ToString());
			FString NewHeaderString = FString::Printf(TEXT("%s.h"), *RenameClass[i]);
			FString NewSourceString = FString::Printf(TEXT("%s.cpp"), *RenameClass[i]);
			XmlFindRename(TCHAR_TO_ANSI(*ProjectFileName), TCHAR_TO_ANSI(*OldHeaderString), TCHAR_TO_ANSI(*NewHeaderString));
			XmlFindRename(TCHAR_TO_ANSI(*ProjectFileName), TCHAR_TO_ANSI(*OldSourceString), TCHAR_TO_ANSI(*NewSourceString));
			XmlFindRename(TCHAR_TO_ANSI(*ProjectFileFilterName), TCHAR_TO_ANSI(*OldHeaderString), TCHAR_TO_ANSI(*NewHeaderString));
			XmlFindRename(TCHAR_TO_ANSI(*ProjectFileFilterName), TCHAR_TO_ANSI(*OldSourceString), TCHAR_TO_ANSI(*NewSourceString));
		}
	}

	for (int i = 0; i < PathToExploresHeader.Num(); i++)
	{
		if (ChangeClick[i] == true)
		{
			FFileHelper::SaveStringToFile(NewTextBlockHeader[i]->GetText().ToString(), *PathToExploresHeader[i]);
			FFileHelper::SaveStringToFile(NewTextBlockSource[i]->GetText().ToString(), *PathToExploresSource[i]);
		}
	}

	//Class File Rename
	for (int i = 0; i < PathToExploresHeader.Num(); i++)
	{
		if (ChangeClick[i] == true)
		{
			FString RenameHeader = PathToExploresHeader[i].Replace(*(SelectAssetDatas[i].AssetName.ToString()), *RenameClass[i]);
			RenameFile(TCHAR_TO_ANSI(*(PathToExploresHeader[i])), TCHAR_TO_ANSI(*RenameHeader));
			FString RenameSource = PathToExploresSource[i].Replace(*(SelectAssetDatas[i].AssetName.ToString()), *RenameClass[i]);
			RenameFile(TCHAR_TO_ANSI(*(PathToExploresSource[i])), TCHAR_TO_ANSI(*RenameSource));
		}
	}

	ReBuildAndRestart();
}

void FClassToolsModule::RenameDlgAllClose()
{
	if (RenameDialog.IsValid() == true)
	{
		RenameDialog.Reset();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FClassToolsModule, ClassTools)