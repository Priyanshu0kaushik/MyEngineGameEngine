//
//  EditorContext.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 10/11/2025.
//

#include "EditorContext.h"
#include "EngineContext.h"
#include "MessageQueue.h"
#include "Scene.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
#include "GLAD/include/glad/glad.h"
#include "glfw3.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "UI/HierarchyPanel.h"
#include "UI/InspectorPanel.h"
#include "UI/ContentBrowserPanel.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <iostream>
#include <string>


EditorContext::EditorContext(){
    
}

void EditorContext::Init(GLFWwindow* aWindow, EngineContext* engine){
    if(engine) m_EngineContext = engine;
    m_Coordinator = engine->GetCoordinator();
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO(); (void)io;
    // Enable docking
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Optionally enable multi-viewport (separate OS windows)
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
//    ImGui::StyleColorsDark(); // or StyleColorsClassic()
    
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]         = ImVec4(0.10f, 0.105f, 0.11f, 1.00f);
    colors[ImGuiCol_Header]           = ImVec4(0.20f, 0.205f, 0.21f, 1.00f);
    colors[ImGuiCol_HeaderHovered]    = ImVec4(0.25f, 0.255f, 0.26f, 1.00f);
    colors[ImGuiCol_HeaderActive]     = ImVec4(0.30f, 0.305f, 0.31f, 1.00f);

    colors[ImGuiCol_Button]           = ImVec4(0.20f, 0.205f, 0.21f, 1.00f);
    colors[ImGuiCol_ButtonHovered]    = ImVec4(0.25f, 0.255f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonActive]     = ImVec4(0.15f, 0.150f, 0.155f, 1.00f);

    colors[ImGuiCol_FrameBg]          = ImVec4(0.20f, 0.205f, 0.21f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.25f, 0.255f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgActive]    = ImVec4(0.30f, 0.305f, 0.31f, 1.00f);

    colors[ImGuiCol_Tab]              = ImVec4(0.15f, 0.150f, 0.155f, 1.00f);
    colors[ImGuiCol_TabHovered]       = ImVec4(0.38f, 0.380f, 0.385f, 1.00f);
    colors[ImGuiCol_TabActive]        = ImVec4(0.28f, 0.280f, 0.285f, 1.00f);
    colors[ImGuiCol_TabUnfocused]     = ImVec4(0.15f, 0.150f, 0.155f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.205f, 0.21f, 1.00f);
    
    colors[ImGuiCol_TitleBg]            = ImVec4(0.10f, 0.105f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.15f, 0.155f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.10f, 0.105f, 0.11f, 1.00f);

    // Docking
    colors[ImGuiCol_DockingPreview]     = ImVec4(0.28f, 0.280f, 0.285f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg]     = ImVec4(0.10f, 0.105f, 0.11f, 1.00f);

    // Menu bar
    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.14f, 0.145f, 0.15f, 1.00f);

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(aWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    
    // UIs Panels Init
    context.coordinator = m_Coordinator;
    context.engine = m_EngineContext;
    context.selectedEntity = &m_SelectedEntity;
    UIPanels.push_back(new HierarchyPanel());
    UIPanels.push_back(new InspectorPanel());
    UIPanels.push_back(new ContentBrowserPanel());
    
    for(UIPanel* UI : UIPanels){
        UI->Init(context);
    }
    
}

void EditorContext::BeginFrame(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool dockspaceOpen = true;
    static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("DockSpace", &dockspaceOpen, windowFlags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

    ImGui::End();
}
void EditorContext::Render(){
    ShowViewport();

    for(UIPanel* UI : UIPanels){
        UI->Draw(context);
    }
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if(io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}


void EditorContext::EndFrame(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorContext::ShowViewport(){
    ImGui::Begin("Viewport");


    ImVec2 viewportMin = ImGui::GetCursorScreenPos();
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    ImGui::Image((void*)(intptr_t)m_EngineContext->GetViewportTexture(), viewportSize, ImVec2(0,1), ImVec2(1,0));

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImGui::SetWindowFocus();
    }
    
    DrawGizmos(viewportMin, viewportSize);

    
    if (!bCameraCapturing) ProcessGizmosInput();

    bool viewportFocused = ImGui::IsWindowHovered();
    bool rightMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);
    
    if (!bCameraCapturing && viewportFocused && rightMouseDown) {
        bCameraCapturing = true;
        m_EngineContext->OnStartControlCam();
        glfwSetInputMode(m_EngineContext->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    }
    else if (!rightMouseDown && bCameraCapturing) {
        bCameraCapturing = false;
        m_EngineContext->OnReleaseCamControl();
        glfwSetInputMode(m_EngineContext->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    DisplayFPS();
    ImGui::End();
}

void EditorContext::DisplayFPS(){
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    int FPS=0;
    if(m_EngineContext) FPS = m_EngineContext->GetFPS();
    std::string text = "FPS: " + std::to_string(FPS);
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

    ImVec2 padding{20.0f,30.f};

    ImVec2 pos = ImVec2(
        windowPos.x + windowSize.x - textSize.x - padding.x,
        windowPos.y + padding.y
    );

    drawList->AddText(pos, IM_COL32(255, 255, 255, 255), text.c_str());
}

void EditorContext::ProcessGizmosInput(){
    if (!ImGui::IsAnyItemActive()) {
        if (ImGui::IsKeyPressed(ImGuiKey_W))
            m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_E))
            m_CurrentGizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R))
            m_CurrentGizmoOperation = ImGuizmo::SCALE;
    }
}

void EditorContext::DrawGizmos(ImVec2 pos, ImVec2 size) {
    if(m_SelectedEntity == UINT32_MAX) return;
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.MousePos.x < -1e30f || io.MousePos.y < -1e30f) return;
    ImGuizmo::BeginFrame();
    
    ImGuizmo::SetOrthographic(false);
    
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

    ImGuizmo::AllowAxisFlip(true);

    const glm::mat4& cameraView = m_EngineContext->GetCameraSystem()->GetView();
    const glm::mat4& cameraProj = m_EngineContext->GetCameraSystem()->GetCameraProjection();

    TransformComponent* tc = m_Coordinator->GetComponent<TransformComponent>(m_SelectedEntity);

    glm::mat4 rotationM = glm::eulerAngleYXZ(glm::radians(tc->rotation.y),
                                             glm::radians(tc->rotation.x),
                                             glm::radians(tc->rotation.z));

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), tc->position) * rotationM * glm::scale(glm::mat4(1.0f), tc->scale);
    glm::mat4 deltaMatrix = glm::mat4(1.0f);

    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
                         m_CurrentGizmoOperation, ImGuizmo::WORLD,
                         glm::value_ptr(modelMatrix), glm::value_ptr(deltaMatrix));
    
    if (ImGuizmo::IsUsing()) {
        float dTr[3], dRo[3], dSc[3];
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(deltaMatrix), dTr, dRo, dSc);

        if (m_CurrentGizmoOperation == ImGuizmo::TRANSLATE) {
            tc->position += glm::vec3(dTr[0], dTr[1], dTr[2]);
        }
        
        if (m_CurrentGizmoOperation == ImGuizmo::ROTATE) {
            tc->rotation += glm::vec3(dRo[0], dRo[1], dRo[2]);
            tc->rotation.x = fmod(tc->rotation.x, 360.0f);
            tc->rotation.y = fmod(tc->rotation.y, 360.0f);
            tc->rotation.z = fmod(tc->rotation.z, 360.0f);

            if (tc->rotation.x < 0) tc->rotation.x += 360.0f;
            if (tc->rotation.y < 0) tc->rotation.y += 360.0f;
            if (tc->rotation.z < 0) tc->rotation.z += 360.0f;
        }
        
        if (m_CurrentGizmoOperation == ImGuizmo::SCALE) {
            tc->scale *= glm::vec3(dSc[0], dSc[1], dSc[2]);
        }
    }
}
