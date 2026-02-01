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
    
    template<typename T>
    void RemoveComponentButton()
    {
        if(ImGui::Button("Remove Component"))
        {
            m_Context.coordinator->RemoveComponent<T>(*m_Context.selectedEntity);
        }
        
    }
private:
    EditorDrawContext m_Context;
};
