//
//  InspectorPanel.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 18/12/2025.
//

#include "InspectorPanel.h"
#include "AssetManager.h"
#include "Scene.h"

void InspectorPanel::Draw(EditorDrawContext &context)
{
    m_Context = context;
    if(*m_Context.selectedEntity == UINT32_MAX) return;
    ImGui::Begin("Inspector");

    
    if (context.coordinator->DoesEntityExist(*context.selectedEntity))
    {
        ShowNameComponent();
        ShowTransformComponent();
        ShowCameraComponent();
        ShowMeshComponent();
        ShowLoadAssetButton();
        
        if(ImGui::Button("Add Component"))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }
        
    }
    ShowAddComponentsList();

    ImGui::End();
}


void InspectorPanel::ShowAddComponentsList()
{
    if(ImGui::BeginPopup("AddComponentPopup"))
    {
        ImGui::Text("Select Component");
        ImGui::Separator();
        static char search[64] = "";
        ImGui::InputText("Search", search, IM_ARRAYSIZE(search));

        for (std::string name : m_Context.coordinator->GetComponentNames())
        {
            if (strstr(name.c_str(), search)!=nullptr)
            {
                if (ImGui::Selectable(name.c_str()))
                {
                    m_Context.coordinator->AddComponentByName(*m_Context.selectedEntity, name.c_str());
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::EndPopup();
    }
}

void InspectorPanel::ShowNameComponent()
{
    NameComponent* nameComponent = m_Context.coordinator->GetComponent<NameComponent>(*m_Context.selectedEntity);
    if(!nameComponent) return;
    ImGui::Text("Name");
    RenameRender();
    ImGui::Separator();

}
void InspectorPanel::ShowTransformComponent()
{
    TransformComponent* tc = m_Context.coordinator->GetComponent<TransformComponent>(*m_Context.selectedEntity);
    if(!tc) return;
    ImGui::SeparatorText("Transform Component");
    glm::vec3 pos = tc->position;
    ImGui::Text("Position");
    if (ImGui::DragFloat3("###Position", &pos.x, 0.1f))
        tc->position=pos;

    glm::vec3 rot = tc->rotation;
    ImGui::Text("Rotation");
    if (ImGui::DragFloat3("###Rotation", &rot.x, 0.1f))
        tc->rotation = rot;

    glm::vec3 scale = tc->scale;
    ImGui::Text("Scale");
    if (ImGui::DragFloat3("###Scale", &scale.x, 0.1f, 0.01f, 10.0f))
        tc->scale = scale;

    ImGui::Separator();

}
void InspectorPanel::ShowMeshComponent()
{
    if(!m_Context.coordinator->GetComponent<MeshComponent>(*m_Context.selectedEntity)) return;
    
    MeshComponent* mesh = m_Context.coordinator->GetComponent<MeshComponent>(*m_Context.selectedEntity);
    ImGui::SeparatorText("Mesh Component");

    auto& allMeshes = AssetManager::Get().GetMeshManager().GetAllMeshes();

    std::string currentMeshName = "Unknown";
    
    for (auto& [path, id] : allMeshes)
    {
        if (id == mesh->meshID)
        {
            currentMeshName = path;
            break;
        }
    }

    ImGui::Text("Current Mesh:");
    ImGui::TextWrapped("%s", currentMeshName.c_str());

    ImGui::Spacing();
    ImGui::TextWrapped("Change Mesh");
    if (ImGui::BeginCombo("###Change Mesh", currentMeshName.c_str()))
    {
        for (auto& [path, id] : allMeshes)
        {
            bool isSelected = (id == mesh->meshID);
            if (ImGui::Selectable(path.c_str(), isSelected))
            {
                mesh->meshID = id;
                mesh->uploaded = false;
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    
    ImGui::Separator();
    
    auto& Textures = AssetManager::Get().GetTextureManager().GetAllTextures();

    std::string currentTextureName = "Unknown";
    
    for (auto& [path, id] : Textures)
    {
        if (id == mesh->textureID)
        {
            currentTextureName = path;
            break;
        }
    }

    ImGui::Text("Current Texture:");
    ImGui::TextWrapped("%s", currentTextureName.c_str());

    ImGui::Spacing();
    ImGui::TextWrapped("Change Texture");
    if (ImGui::BeginCombo("###Change Texture", currentTextureName.c_str()))
    {
        for (auto& [path, id] : Textures)
        {
            bool isSelected = (id == mesh->textureID);
            if (ImGui::Selectable(path.c_str(), isSelected))
            {
                mesh->textureID = id;
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    if(ImGui::Button("Reload Shader")){
        m_Context.engine->GetShader()->Reload();
    }
    
    ImGui::Separator();

}

void InspectorPanel::ShowCameraComponent()
{
    CameraComponent* cam = m_Context.coordinator->GetComponent<CameraComponent>(*m_Context.selectedEntity);
    if(!cam) return;
    ImGui::SeparatorText("Camera Component");
    float fov = cam->Fov;
    ImGui::Text("FOV");
    if (ImGui::DragFloat("###FOV", &fov, 0.1f))
        cam->Fov=fov;

    float near = cam->Near;
    ImGui::Text("Near");
    if (ImGui::DragFloat("###Near", &near, 0.1f))
        cam->Near=near;

    float far = cam->Far;
    ImGui::Text("Far");
    if (ImGui::DragFloat("###Far", &far, 0.1f))
        cam->Far=far;
    
    ImGui::Text("Is Primary");
    bool IsMain = cam->IsPrimary;
    if(ImGui::Checkbox("###MainCam", &IsMain))
        cam->IsPrimary = IsMain;

    
    ImGui::Separator();
    
}

void InspectorPanel::RenameRender()
{

    NameComponent* nc = m_Context.coordinator->GetComponent<NameComponent>(*m_Context.selectedEntity);
    if(nc){
        char buffer[128];

        strcpy(buffer, nc->Name.c_str());
        bool enterPressed =
            ImGui::InputText(
                "###Name",
                 buffer,
                IM_ARRAYSIZE(buffer),
                ImGuiInputTextFlags_EnterReturnsTrue
            );
        if (enterPressed || ImGui::IsItemDeactivatedAfterEdit())
        {
            std::string trimmed = buffer;
            trimmed.erase(0, trimmed.find_first_not_of(" \t\n"));
            trimmed.erase(trimmed.find_last_not_of(" \t\n") + 1);
            
            strcpy(buffer, trimmed.c_str());
            NameComponent* nameComponent = m_Context.coordinator->GetComponent<NameComponent>(*m_Context.selectedEntity);
            if(m_Context.coordinator->DoesEntityExist(*m_Context.selectedEntity) && !trimmed.empty() &&
               trimmed !=nameComponent->Name.c_str()){
                if(m_Context.engine) m_Context.engine->GetScene()->RenameEntity(*m_Context.selectedEntity, trimmed.c_str());
            }
            else strcpy(buffer, nameComponent->Name.c_str());
        }
    }
    
}

void InspectorPanel::ShowLoadAssetButton()
{
    ImGui::SeparatorText("Load Asset");
    char m_AssetPath[128] = "Path";
    bool pathEntered = ImGui::InputText(
        "###AssetPath",
        m_AssetPath,
        IM_ARRAYSIZE(m_AssetPath),
        ImGuiInputTextFlags_EnterReturnsTrue
    );

    if(pathEntered) AssetManager::Get().GetAsset(m_AssetPath);
}
