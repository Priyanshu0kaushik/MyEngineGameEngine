//
//  InspectorPanel.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 18/12/2025.
//

#include "UI/InspectorPanel.h"
#include "AssetManager.h"
#include <glm/gtc/type_ptr.hpp>
#include "Scene.h"
#include "ShaderManager.h"
#include <filesystem>

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
        ShowLightComponent();
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

void InspectorPanel::ShowLightComponent()
{
    LightComponent* light = m_Context.coordinator->GetComponent<LightComponent>(*m_Context.selectedEntity);
    if (light) {
        if (ImGui::TreeNodeEx((void*)typeid(LightComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Light Component")) {
            
            const char* lightTypes[] = { "Directional", "Point", "Spot" };
            int currentType = (int)light->type;
            if (ImGui::Combo("Light Type", &currentType, lightTypes, IM_ARRAYSIZE(lightTypes))) {
                light->type = (LightType)currentType;
            }

            ImGui::Separator();

            ImGui::ColorEdit3("Light Color", glm::value_ptr(light->color));
            ImGui::DragFloat("Intensity", &light->intensity, 0.1f, 0.0f, 100.0f);

            if (light->type != LightType::Directional) {
                ImGui::Text("Attenuation Factors");
                ImGui::DragFloat("Constant", &light->constant, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("Linear", &light->linear, 0.001f, 0.0f, 1.0f);
                ImGui::DragFloat("Quadratic", &light->quadratic, 0.0001f, 0.0f, 1.0f);
            }

            if (light->type == LightType::Spot) {
                ImGui::Text("Spotlight Settings");
            }

            ImGui::TreePop();
        }
    }
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
    ShowMaterialSetting(mesh->material);
    if(ImGui::Button("Reload Shader")){
        m_Context.engine->GetShaderManager()->ReloadShaders();
    }
    
    
}

void InspectorPanel::ShowMaterialSetting(Material& material)
{
    auto& Textures = AssetManager::Get().GetTextureManager().GetAllTextures();

    DrawAssetSlot("Main Texture", material.albedoPath, material.albedoID);
    DrawAssetSlot("Normal Map", material.normalPath, material.normalID);
    DrawAssetSlot("Specular Map", material.specPath, material.specID);
    
    UpdateAssetSlot(material.albedoPath, material.albedoID);
    UpdateAssetSlot(material.normalPath, material.normalID);
    UpdateAssetSlot(material.specPath, material.specID);
    
    ImGui::Separator();

    
    if (ImGui::CollapsingHeader("Material")) {
        ImGui::ColorEdit3("Ambient", glm::value_ptr(material.Ambient));
        ImGui::ColorEdit3("Diffuse", glm::value_ptr(material.Diffuse));
        ImGui::ColorEdit3("Specular", glm::value_ptr(material.Specular));
        ImGui::SliderFloat("Shininess", &material.Shininess, 1.0f, 256.0f);
    }
    
}

void InspectorPanel::DrawAssetSlot(const char* Name, std::string &path, uint32_t &iD)
{
    ImGui::PushID(Name);
    std::string currentTextureName = !path.empty()? path : "Unknown";
    
    ImGui::TextWrapped("%s", Name);

    ImGui::Spacing();
    ImGui::TextWrapped("Change Texture");

    uint32_t openGLHandle = 0;
    if (iD != UINT32_MAX) {
        AssetHandle handle = AssetManager::Get().GetTextureManager().GetTexture(iD);
        if (handle.IsReady && handle.Data) {
            TextureData* texData = static_cast<TextureData*>(handle.Data);
            openGLHandle = texData->TextureObject;
        }
    }
    if (iD != UINT32_MAX) {
        ImGui::Image((ImTextureID)(uintptr_t)openGLHandle, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
    } else {
        ImGui::Button(path.empty() ? "Empty" : "Loading...", ImVec2(64, 64));
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            const char* assetPath = (const char*)payload->Data;
            
            path = assetPath;
            iD = UINT32_MAX;
            AssetManager::Get().GetAsset(path);
            std::cout << "Dropped and Loaded: " << path << std::endl;
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::SameLine();
    ImGui::TextWrapped("%s", path.empty() ? "None" : std::filesystem::path(path).filename().string().c_str());
    ImGui::PopID();
    
}

void InspectorPanel::UpdateAssetSlot(std::string &path, uint32_t &id)
{
    if (!path.empty() && id == UINT32_MAX) {
        AssetHandle handle = AssetManager::Get().GetAsset(path);
        if (handle.IsReady) {
            id = handle.iD;
        }
    }
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
