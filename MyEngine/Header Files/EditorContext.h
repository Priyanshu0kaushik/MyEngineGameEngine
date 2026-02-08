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
    void RenderEditor();
    void RenderLauncher();
    void SetGizmosOperation(ImGuizmo::OPERATION newGizmoOperation);
    void OnEditorLaunched();

private:
    void DrawLauncher();
    void ProcessGizmosInput();
    void ShowViewport();
    void ViewportToolbar();
    void DrawGizmos(ImVec2 pos, ImVec2 size);
    void DisplayFPS();
    std::string OpenFolderDialog();
private:
    EditorDrawContext context;
    
    ImGuiIO* io = nullptr;
    ImFont* m_LauncherFont = nullptr;
    EngineContext* m_EngineContext = nullptr;
    Entity m_SelectedEntity;
    Coordinator* m_Coordinator = nullptr;
    
    ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
    std::vector<UIPanel*> UIPanels;
    
    bool bCameraCapturing = false;
    bool m_ShowNewProjectPopup = false;

};
