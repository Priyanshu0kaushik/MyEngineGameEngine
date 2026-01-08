//
//  HierarchyPanel.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 18/12/2025.
//

#include "UI/HierarchyPanel.h"

void HierarchyPanel::Draw(EditorDrawContext& context)
{
    ImGui::Begin("Hierarchy");
    const std::vector<Entity>& aliveEntities = context.coordinator->GetAliveEntities();
    

    for (size_t i = 0; i < aliveEntities.size(); i++)
    {
        Entity e = aliveEntities[i];
        std::string label = "EntityNameDefault";
        if(NameComponent* nC = context.coordinator->GetComponent<NameComponent>(e)) label = nC->Name;

        bool isSelected = (*context.selectedEntity == e);
        if (ImGui::Selectable(label.c_str(), isSelected))
        {
            if(context.coordinator->DoesEntityExist(e)) *context.selectedEntity = e;
            
        }
        if (ImGui::BeginPopupContextItem(("object_context_menu##" + std::to_string(e)).c_str())) {
            if (ImGui::MenuItem("Delete"))
            {
                if(context.engine){
                    context.engine->DeleteEntity(e);
                    *context.selectedEntity = UINT32_MAX;
                }
            }
            ImGui::EndPopup();
        }

    }
    if (ImGui::Button("Add GameObject"))
    {
        *context.selectedEntity =  context.engine->CreateEntity("GameObject");
    }
    ImGui::End();
}
