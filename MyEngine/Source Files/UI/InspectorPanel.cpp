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
#include <algorithm>


void InspectorPanel::Draw(EditorDrawContext &context)
{
    m_Context = context;
    if(*m_Context.selectedEntity == UINT32_MAX) return;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    ImGui::Begin("Inspector");

    
    if (context.coordinator->DoesEntityExist(*context.selectedEntity))
    {
        ShowNameComponent();
        ImGui::Spacing();
        ShowTransformComponent();
        ImGui::Spacing();
        ShowCameraComponent();
        ImGui::Spacing();
        ShowMeshComponent();
        ImGui::Spacing();
        ShowRigidBodyComponent();
        ImGui::Spacing();
        ShowBoxColliderComponent();
        ImGui::Spacing();
        ShowSphereColliderComponent();
        ImGui::Spacing();
        ShowLightComponent();
        ImGui::Spacing();
        ShowScriptComponent();
        
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.2f, 1.0f));
        if(ImGui::Button(" + Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 30)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }
        ImGui::PopStyleColor();
        
        ShowAddComponentsList();
    }
        
    ImGui::Separator();
    
    float footerHeight = 30.0f;
    if (ImGui::GetContentRegionAvail().y > footerHeight)
    {
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - footerHeight - ImGui::GetStyle().WindowPadding.y);
    }
    if(ImGui::Button("Reload Shader", ImVec2(ImGui::GetContentRegionAvail().x, footerHeight - 5.0f))) {
        m_Context.engine->GetShaderManager()->ReloadShaders();
    }
    ImGui::PopStyleVar();
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
            if(name == "Collider Component") continue;
            if (strstr(name.c_str(), search)!=nullptr)
            {
                if (ImGui::Selectable(name.c_str()))
                {
                    if(name == "Box Collider") {
                        m_Context.coordinator->AddComponentByName(*m_Context.selectedEntity, "Collider Component");
                        m_Context.coordinator->GetComponent<ColliderComponent>(*m_Context.selectedEntity)->type = ColliderType::Box;
                    }
                    else if(name == "Sphere Collider") {
                        m_Context.coordinator->AddComponentByName(*m_Context.selectedEntity, "Collider Component");
                        m_Context.coordinator->GetComponent<ColliderComponent>(*m_Context.selectedEntity)->type = ColliderType::Sphere;
                    }
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
    if (ImGui::CollapsingHeader(" Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Spacing();
        
        glm::vec3 pos = tc->position;
        ImGui::Text("Position");
        if (ImGui::DragFloat3("###Position", &pos.x, 0.1f)){
            tc->SetPosition(pos);
        }

        glm::vec3 rot = tc->rotation;
        ImGui::Text("Rotation");
        if (ImGui::DragFloat3("###Rotation", &rot.x, 0.1f)){
            tc->SetRotation(rot);
        }
        glm::vec3 scale = tc->scale;
        ImGui::Text("Scale");
        if (ImGui::DragFloat3("###Scale", &scale.x, 0.1f, 0.01f, 10.0f)){
            tc->SetScale(scale);
        }
        ImGui::Spacing();
    }

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
        RemoveComponentButton<LightComponent>();
        ImGui::Separator();

    }
}

void InspectorPanel::ShowMeshComponent()
{
    if(!m_Context.coordinator->GetComponent<MeshComponent>(*m_Context.selectedEntity)) return;
    
    MeshComponent* mesh = m_Context.coordinator->GetComponent<MeshComponent>(*m_Context.selectedEntity);
    
    if (ImGui::CollapsingHeader("Mesh Component", ImGuiTreeNodeFlags_DefaultOpen)){
        
        DrawAssetSlot("Mesh",mesh->meshPath , mesh->meshID, AssetType::Mesh);
        bool MeshLoaded = UpdateAssetSlot(mesh->meshPath, mesh->meshID);
        
        Mesh* meshData = static_cast<Mesh*>(AssetManager::Get().GetAsset(AssetType::Mesh, mesh->meshID).Data);
        
        if(MeshLoaded && meshData) meshData->uploaded = false;
        
        
        ShowMaterialSetting(mesh->material);
        
        ImGui::Spacing();
        const char* mipmapModes[] = { "Nearest", "Linear", "Trilinear (Mipmaps)" };
        static int currentMode = 2;
        
        ImGui::Text("MipMap Settings");
        
        if (ImGui::Combo("###MipMapMode", &currentMode, mipmapModes, IM_ARRAYSIZE(mipmapModes))) {
            if(mesh->material.albedoID != UINT32_MAX) {
                AssetManager::Get().GetTextureManager().SetMipMapSettings(mesh->material.albedoID, currentMode);
            }
        }
        
        ImGui::Spacing();
        RemoveComponentButton<MeshComponent>();
        ImGui::Separator();
    }
    
}

void InspectorPanel::ShowScriptComponent()
{
    ScriptComponent* script = m_Context.coordinator->GetComponent<ScriptComponent>(*m_Context.selectedEntity);
    if(!script) return;
    
    if (ImGui::CollapsingHeader("Script Component", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::BeginGroup();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        
        ImGui::Spacing();
        
        ImGui::TextWrapped("Change Script");
        
        ImGui::Button(script->scriptPath.empty() ? "None" : "Script", ImVec2(64, 64));
        
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const char* assetPath = (const char*)payload->Data;
                
                std::filesystem::path filePath(assetPath);
                if (!filePath.has_extension()) return;
                
                std::string lowerStr = filePath.extension().string();
                std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
                std::cout<< lowerStr<<std::endl;
                if(lowerStr == ".lua") script->scriptPath = assetPath;
                
                std::cout << "Dropped : " << script->scriptPath << std::endl;
            }
            ImGui::EndDragDropTarget();
            
        }
        

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        ImGui::EndGroup();
        
        ImGui::Spacing();
        RemoveComponentButton<ScriptComponent>();
        ImGui::Separator();
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

    ImGui::Spacing();
    RemoveComponentButton<CameraComponent>();
    ImGui::Separator();
    
}

void InspectorPanel::ShowRigidBodyComponent()
{
    RigidBodyComponent* rigidBody = m_Context.coordinator->GetComponent<RigidBodyComponent>(*m_Context.selectedEntity);
    if(!rigidBody) return;
    if (ImGui::CollapsingHeader("RigidBody Component", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat("Mass", &rigidBody->mass, 0.1f, 0.0f, 1000.0f);
        ImGui::DragFloat("Gravity", &rigidBody->gravityScale, 0.1f, 0.0f, 1.0f);
        
        ImGui::Checkbox("Static", &rigidBody->isStatic);
        ImGui::Checkbox("Kinematic", &rigidBody->isKinematic);
        
        ImGui::Spacing();
        RemoveComponentButton<RigidBodyComponent>();
        ImGui::Separator();
    }
}

void InspectorPanel::ShowBoxColliderComponent()
{
    ColliderComponent* collider = m_Context.coordinator->GetComponent<ColliderComponent>(*m_Context.selectedEntity);
    BoxColliderComponent* boxCollider = m_Context.coordinator->GetComponent<BoxColliderComponent>(*m_Context.selectedEntity);
    if(!collider|| !boxCollider) return;
    if (ImGui::CollapsingHeader("Box Collider", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat("Friction", &collider->friction, 0.1f, 0.0f, 1.0f);
        ImGui::DragFloat("Bounciness", &collider->bounciness, 0.1f, 0.0f, 1.0f);
        
        if(ImGui::DragFloat3("Center", &collider->center.x, 0.1f)){
            collider->isDirty = true;
        }
        if(ImGui::DragFloat3("Extents", &boxCollider->extents.x, 0.1f)){
            collider->isDirty = true;
        }
    
        ImGui::Spacing();
        RemoveComponentButton<BoxColliderComponent>();
        ImGui::Separator();
    }
}
void InspectorPanel::ShowSphereColliderComponent()
{
    ColliderComponent* collider = m_Context.coordinator->GetComponent<ColliderComponent>(*m_Context.selectedEntity);
    SphereColliderComponent* sphereCollider = m_Context.coordinator->GetComponent<SphereColliderComponent>(*m_Context.selectedEntity);
    if(!collider|| !sphereCollider) return;
    if (ImGui::CollapsingHeader("Sphere Collider", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat("Friction", &collider->friction, 0.1f, 0.0f, 1.0f);
        ImGui::DragFloat("Bounciness", &collider->bounciness, 0.1f, 0.0f, 1.0f);
        
        if(ImGui::DragFloat3("Center", &collider->center.x, 0.1f)){
            collider->isDirty = true;
        }
        if(ImGui::DragFloat("Radius", &sphereCollider->radius, 0.1f)){
            collider->isDirty = true;
        }
    
        ImGui::Spacing();
        RemoveComponentButton<SphereColliderComponent>();
        ImGui::Separator();
    }
}

void InspectorPanel::ShowMaterialSetting(Material& material)
{
    auto& Textures = AssetManager::Get().GetTextureManager().GetAllTextures();

    DrawAssetSlot("Main Texture", material.albedoPath, material.albedoID, AssetType::Texture);
    DrawAssetSlot("Normal Map", material.normalPath, material.normalID, AssetType::Texture);
    DrawAssetSlot("Specular Map", material.specPath, material.specID, AssetType::Texture);
    
    UpdateAssetSlot(material.albedoPath, material.albedoID);
    UpdateAssetSlot(material.normalPath, material.normalID);
    UpdateAssetSlot(material.specPath, material.specID);
    
    ImGui::Spacing();
    ImGui::Text("Material");
    ImGui::Spacing();
    ImGui::ColorEdit3("Ambient", glm::value_ptr(material.Ambient));
    ImGui::ColorEdit3("Diffuse", glm::value_ptr(material.Diffuse));
    ImGui::ColorEdit3("Specular", glm::value_ptr(material.Specular));
    ImGui::SliderFloat("Shininess", &material.Shininess, 1.0f, 256.0f);
    
}

void InspectorPanel::DrawAssetSlot(const char* Name, std::string &path, uint32_t &iD, AssetType Type)
{
    ImGui::PushID(Name);
    ImGui::BeginGroup();
    
    ImGui::TextDisabled("%s", Name);
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    ImGui::Spacing();
    
    uint32_t openGLHandle = 0;
    switch(Type){
        case AssetType::Mesh:
            ImGui::TextWrapped("Change Mesh");
            break;
        case AssetType::Texture:
            ImGui::TextWrapped("Change Texture");
            if (iD != UINT32_MAX) {
                AssetHandle handle = AssetManager::Get().GetTextureManager().GetTexture(iD);
                if (handle.IsReady && handle.Data) {
                    TextureData* texData = static_cast<TextureData*>(handle.Data);
                    openGLHandle = texData->TextureObject;
                }
            }
            break;
        default:
            ImGui::TextWrapped("Change Asset");
    }

    if (Type == AssetType::Texture && iD != UINT32_MAX) {
        ImGui::Image((ImTextureID)(uintptr_t)openGLHandle, ImVec2(40, 40), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1,1,1,1), ImVec4(1,1,1,0.2f));
    }
    else {
        ImGui::Button(path.empty() ? "None" : "FILE", ImVec2(64, 64));
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            const char* assetPath = (const char*)payload->Data;
            if (!path.empty() && path != assetPath)
            {
                 AssetManager::Get().RemoveAssetReference(path, Type);
            }
            
            path = assetPath;
            iD = UINT32_MAX;
            AssetManager::Get().GetAsset(path);
            std::cout << "Dropped and Loaded: " << path << std::endl;
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::SameLine();
    ImGui::BeginChild(Name, ImVec2(0, 40), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextWrapped("%s", path.empty() ? "Drop Asset Here" : std::filesystem::path(path).filename().string().c_str());
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", path.c_str());
    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::EndGroup();
    ImGui::PopID();
    
}

bool InspectorPanel::UpdateAssetSlot(std::string &path, uint32_t &id)
{
    if (!path.empty() && id == UINT32_MAX) {
        AssetHandle handle = AssetManager::Get().GetAsset(path);
        if (handle.IsReady && handle.iD != UINT32_MAX) {
            std::cout<<handle.iD<<std::endl;
            id = handle.iD;
            AssetType type = AssetManager::Get().GetAssetTypeFromExtension(path);
            AssetManager::Get().AddAssetReference(path, type);
            return true;
        }
    }
    return false;
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

void InspectorPanel::RemoveReference(const std::string &path, AssetType type) { 
    AssetManager::Get().RemoveAssetReference(path, type);
}

