//
//  ContentBrowserPanel.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 04/01/2026.
//

#include "ContentBrowserPanel.h"
#include "Project.h"


void ContentBrowserPanel::Init(EditorDrawContext& context)
{
    
    FolderIconData = AssetManager::Get().GetAsset("EngineAssets/Images/FolderIcon.png");
    FolderIconData = AssetManager::Get().GetAsset("EngineAssets/Images/FileIcon.png");
}

void ContentBrowserPanel::Draw(EditorDrawContext& context)
{
    if(!Project::GetActive()) return;
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
        
        
        ImGui::PushID(filenameString.c_str());

        bool isDirectory = directoryEntry.is_directory();
        
        if (isDirectory)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.7f, 0.2f, 1.0f));
        } else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        }

        if (FolderIconData.Data && static_cast<TextureData*>(FolderIconData.Data)->IsLoaded && FileIconData.Data && static_cast<TextureData*>(FileIconData.Data)->IsLoaded)
        {
            unsigned int TextureObject;
            if(isDirectory){
                TextureData* Data = static_cast<TextureData*>(FolderIconData.Data);
                TextureObject = Data->TextureObject;
            }
            else{
                TextureData* Data = static_cast<TextureData*>(FileIconData.Data);
                TextureObject = Data->TextureObject;
            }
            if (ImGui::ImageButton(isDirectory ? "[Folder]" : "[File]", TextureObject, ImVec2(64, 64)))
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
        
        
        
        ImGui::PopStyleColor();

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

