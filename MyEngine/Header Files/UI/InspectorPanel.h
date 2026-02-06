//
//  InspectorPanel.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 18/12/2025.
//

#pragma once
#include "UIPanel.h"

class InspectorPanel : public UIPanel{
public:
    virtual void Draw(EditorDrawContext& context) override;
private:
    void ShowAddComponentsList();
    void ShowNameComponent();
    void ShowTransformComponent();
    void ShowMeshComponent();
    void ShowCameraComponent();
    void ShowLoadAssetButton();
    void RenameRender();
    void ShowLightComponent();
    void ShowMaterialSetting(Material& material);
    
    void DrawAssetSlot(const char* Name, std::string& path, uint32_t& iD, AssetType Type);
    bool UpdateAssetSlot(std::string& path, uint32_t& id);
    
    void RemoveReference(const std::string& path, AssetType type);

    template<typename T>
    void RemoveComponentButton()
    {
        ImGui::PushID(typeid(T).name());
        if(ImGui::Button("Remove Component"))
        {
            Entity e = *m_Context.selectedEntity;
            T* component = m_Context.coordinator->GetComponent<T>(e);

            if (component)
            {
                if constexpr (std::is_same_v<T, MeshComponent>)
                {
                    if (!component->meshPath.empty()) {
                        RemoveReference(component->meshPath, AssetType::Mesh);
                    }
                    if(!component->material.albedoPath.empty())
                    {
                        RemoveReference(component->material.albedoPath, AssetType::Texture);
                    }
                    if(!component->material.normalPath.empty())
                    {
                        RemoveReference(component->material.normalPath, AssetType::Texture);
                    }
                    if(!component->material.specPath.empty())
                    {
                        RemoveReference(component->material.specPath, AssetType::Texture);
                    }
                }
                
                
            }

            m_Context.coordinator->RemoveComponent<T>(e);
        }
        ImGui::PopID();
    }
private:
    EditorDrawContext m_Context;
};
