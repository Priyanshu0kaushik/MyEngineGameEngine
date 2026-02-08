//
//  ContentBrowserPanel.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 04/01/2026.
//

#include "UI/ContentBrowserPanel.h"
#include "Project.h"
#include <string>


void ContentBrowserPanel::Init(EditorDrawContext& context)
{
    FolderIconData = AssetManager::Get().GetAsset("EngineAssets/Images/FolderIcon.png");
    FileIconData = AssetManager::Get().GetAsset("EngineAssets/Images/FileIcon.png");
}

void ContentBrowserPanel::Draw(EditorDrawContext& context)
{
    if(!Project::GetActiveProject()) return;
    std::filesystem::path assetPath = Project::GetAssetDirectory();
    if (m_CurrentDirectory.empty()) {
        m_CurrentDirectory = assetPath;
    }

    bool triggerDeletePopup = false;
    
    ImGui::Begin("Content Browser");

    if (m_CurrentDirectory != assetPath)
    {
        if (ImGui::Button("<- Back"))
        {
            m_CurrentDirectory = m_CurrentDirectory.parent_path();
        }
    }
    
    ImGui::SameLine();
    ImGui::Text("Current Path: %s", m_CurrentDirectory.string().c_str());
    ImGui::Separator();

    float padding = 16.0f;
    float thumbnailSize = 80.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, 0, false);
    
    for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
    {
        const auto& path = directoryEntry.path();
        std::string filenameString = path.filename().string();
        std::string extension = path.extension().string();
        if (!directoryEntry.is_directory())
        {
            auto it = std::find(m_ignoredExtensions.begin(), m_ignoredExtensions.end(), extension);
            if (it!=m_ignoredExtensions.end())
            {
                continue;
            }
        }
        
        
        ImGui::PushID(filenameString.c_str());

        bool isDirectory = directoryEntry.is_directory();
        

        if (FolderIconData.Data && static_cast<TextureData*>(FolderIconData.Data)->IsLoaded && FileIconData.Data && static_cast<TextureData*>(FileIconData.Data)->IsLoaded)
        {
            uint32_t textureID = isDirectory ?
                    static_cast<TextureData*>(FolderIconData.Data)->TextureObject :
                    static_cast<TextureData*>(FileIconData.Data)->TextureObject;
            ImTextureID imTexID = (ImTextureID)(intptr_t)textureID;
            
            if (ImGui::ImageButton(isDirectory ? "[Folder]" : "[File]", imTexID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0)))
            {
                if (isDirectory) m_CurrentDirectory /= path.filename();
                else std::cout << "Selected Asset: " << filenameString << std::endl;
            }
        }
        else
        {
            if (ImGui::Button(isDirectory ? "[Folder]" : "[File]", { thumbnailSize, thumbnailSize }))
            {
                if (isDirectory) m_CurrentDirectory /= path.filename();
                else std::cout << "Selected Asset: " << filenameString << std::endl;
                
            }
        }

        if (!isDirectory && ImGui::BeginDragDropSource())
        {
            std::string itemPath = path.string();
            
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), itemPath.size() + 1);
            
            ImGui::Text("%s", filenameString.c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete"))
            {
                s_PathToDelete = directoryEntry.path();
                triggerDeletePopup = true;
            }
            ImGui::EndPopup();
        }
        ImGui::TextWrapped("%s", filenameString.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);
    
    if (triggerDeletePopup)
    {
        ImGui::OpenPopup("DeleteConfirmation");
    }
    
    OnItemDeletePressed();
    OnContentBrowserRightClick();
    
    
    ImGui::End();
}

void ContentBrowserPanel::OnItemDeletePressed()
{
    if (ImGui::BeginPopupModal("DeleteConfirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to delete this item(s)?");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "This action cannot be undone!");
        
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            if (!s_PathToDelete.empty()) {
                std::filesystem::remove_all(s_PathToDelete);
                s_PathToDelete.clear();
            }
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            s_PathToDelete.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ContentBrowserPanel::OnContentBrowserRightClick() { 
    if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("New Folder"))
        {
            std::filesystem::create_directory(m_CurrentDirectory / "NewFolder");
            std::cout << "Created New Folder!" << std::endl;
        }
        ImGui::EndPopup();
    }
}

