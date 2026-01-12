//
//  ContentBrowserPanel.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 04/01/2026.
//

#pragma once
#include "UIPanel.h"
#include <filesystem>
#include "AssetManager.h"

class ContentBrowserPanel: public UIPanel
{
public:
    virtual void Init(EditorDrawContext& context) override;
    virtual void Draw(EditorDrawContext& context) override;
private:
    void ShowItems();
    void OnContentBrowserRightClick();
    void OnItemDeletePressed();
    
    std::filesystem::path s_PathToDelete;

    AssetHandle FolderIconData, FileIconData;
    std::filesystem::path m_CurrentDirectory;
    std::vector<std::string> m_ignoredExtensions = { ".memesh", ".DS_Store"};
};
