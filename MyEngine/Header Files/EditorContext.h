//
//  EditorContext.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 10/11/2025.
//

#pragma once
#include "imgui.h"
#include "ECS/Coordinator.h"
#include "ImGuizmo.h"
#include "UI/UIPanel.h"

class GLFWwindow;
class EngineContext;

class EditorContext{
public:
    EditorContext();
    void Init(GLFWwindow* window, EngineContext* engine);
    
    void BeginFrame();
    void EndFrame();
    void Render();
    void SetGizmosOperation(ImGuizmo::OPERATION newGizmoOperation);
    
private:
    void ProcessGizmosInput();
    void ShowViewport();
    void DrawGizmos(ImVec2 pos, ImVec2 size);
    void DisplayFPS();
private:
    ImGuiIO* io;
    EngineContext* m_EngineContext = nullptr;
    Entity m_SelectedEntity;
    Coordinator* m_Coordinator;
    
    ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
    std::vector<UIPanel*> UIPanels;
    
    bool bCameraCapturing = false;

    EditorDrawContext context;
};
