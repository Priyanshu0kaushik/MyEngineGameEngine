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
    
    void DrawAssetSlot(const char* Name, std::string& path, uint32_t& iD);
    void UpdateAssetSlot(std::string& path, uint32_t& id);
private:
    EditorDrawContext m_Context;
};
