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
#include "GLAD/include/glad/glad.h"
#include "glfw3.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "UI/HierarchyPanel.h"
#include "UI/InspectorPanel.h"
#include "UI/ContentBrowserPanel.h"
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

    ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::Image((void*)(intptr_t)m_EngineContext->GetViewportTexture(), size, ImVec2(0,1), ImVec2(1,0));
    
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
