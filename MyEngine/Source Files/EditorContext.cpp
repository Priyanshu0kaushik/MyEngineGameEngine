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
#include "Project.h"
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
#include <cstdio>
#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
#endif

EditorContext::EditorContext(){
    
}

// INITIALIZATION
// Sets up the ImGui Context, Style (Theme), and Windowing Flags.

void EditorContext::Init(GLFWwindow* aWindow, EngineContext* engine){
    if(engine) m_EngineContext = engine;
    m_Coordinator = engine->GetCoordinator();
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO(); (void)io;
    
    // Enable Docking
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    
    // THEME SETUP

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

    
    // Setup Platform/Renderer bindings (GLFW + OpenGL 3)
    ImGui_ImplGlfw_InitForOpenGL(aWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    
    // Font Load
    io->Fonts->AddFontDefault();
    m_LauncherFont = io->Fonts->AddFontFromFileTTF("EngineAssets/Fonts/Playwrite_NZ_Basic/PlaywriteNZBasic-Light.ttf", 40.0f);
    unsigned char* tex_pixels;
    int tex_width, tex_height;
    io->Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_width, &tex_height);
    
    
}

void EditorContext::OnEditorLaunched(){
    // Initialize UI Panels
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

// BEGIN FRAME
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

void EditorContext::RenderEditor(){
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

void EditorContext::RenderLauncher(){
    DrawLauncher();

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


// VIEWPORT LOGIC
// 1. Renders the Engine's Framebuffer as an Image.
// 2. Handles Input Switching (Mouse vs Camera Control).
// 3. Renders Gizmos.

void EditorContext::ShowViewport(){
    ImGui::Begin("Viewport");

    ViewportToolbar();

    ImVec2 viewportMin = ImGui::GetCursorScreenPos();
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    // RENDER Viewport:
    // Takes the OpenGL Texture ID and draws it as an Image.
    ImGui::Image((void*)(intptr_t)m_EngineContext->GetViewportTexture(), viewportSize, ImVec2(0,1), ImVec2(1,0));

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImGui::SetWindowFocus();
    }
    
    // Draw Transform Gizmos
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
    ImGui::End();
}

void EditorContext::ViewportToolbar()
{
    // Toolbar Start
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 1));
    ImGui::BeginChild("ToolbarChild", ImVec2(0, 28), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    
    if (ImGui::Button("SAVE"))
    {
        std::string activeScenePath = Project::GetActiveAbsoluteScenePath();
        if (!activeScenePath.empty()) {
            m_EngineContext->GetScene()->Save();
        }
    }

    ImGui::SameLine();
    
    float buttonSize = 50.0f;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonSize) * 0.5f);
    
    EngineState m_EngineState = m_EngineContext->GetState();
    bool isPlay = (m_EngineState == EngineState::Play);
    
    if(isPlay) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    
    // Play/Stop Button
    if (ImGui::Button(isPlay ? "STOP" : "PLAY", ImVec2(buttonSize, 15))) {
        if (!isPlay) m_EngineContext->SetState(EngineState::Play);
        else m_EngineContext->SetState(EngineState::Edit);
    }
    DisplayFPS();
    
    ImGui::PopStyleColor();
    ImGui::EndChild();
    ImGui::PopStyleVar();
    // Toolbar End
}

void EditorContext::DisplayFPS(){
    int FPS = m_EngineContext ? m_EngineContext->GetFPS() : 0;
    std::string fpsText = std::to_string(FPS) + " FPS";
    ImVec2 textSize = ImGui::CalcTextSize(fpsText.c_str());

    // to the right
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textSize.x - 15.0f);
    ImGui::SetCursorPosY(10.0f);

    ImVec4 fpsColor = ImVec4(1, 1, 1, 0.6f);
    if (FPS < 30) fpsColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);

    ImGui::TextColored(fpsColor, "%s", fpsText.c_str());
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

// GIZMO RENDERING
// Uses ImGuizmo to manipulate the selected Entity's Transform Component.

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
    
    if(!tc) return;
    glm::mat4 rotationM = glm::eulerAngleYXZ(glm::radians(tc->rotation.y),
                                             glm::radians(tc->rotation.x),
                                             glm::radians(tc->rotation.z));

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), tc->position) * rotationM * glm::scale(glm::mat4(1.0f), tc->scale);
    glm::mat4 deltaMatrix = glm::mat4(1.0f);

    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
                         m_CurrentGizmoOperation, ImGuizmo::WORLD,
                         glm::value_ptr(modelMatrix), glm::value_ptr(deltaMatrix));
    
    if (ImGuizmo::IsUsing()) {
        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix),
                                                     glm::value_ptr(translation),
                                                     glm::value_ptr(rotation),
                                                     glm::value_ptr(scale));

        if (m_CurrentGizmoOperation == ImGuizmo::TRANSLATE) {
            tc->SetPosition(translation);
        }
        else if (m_CurrentGizmoOperation == ImGuizmo::ROTATE) {
            tc->SetRotation(rotation);
        }
        else if (m_CurrentGizmoOperation == ImGuizmo::SCALE) {
            tc->SetScale(scale);
        }
    }
}

// Launcher UI
void EditorContext::DrawLauncher() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("Launcher Background", nullptr, flags);

    float windowWidth = viewport->Size.x;
    float windowHeight = viewport->Size.y;

    ImGui::SetCursorPosY(windowHeight * 0.3f);
    
    
    ImGuiIO& io = ImGui::GetIO();

    if (m_LauncherFont) ImGui::PushFont(m_LauncherFont);

    
    // Engine Title
    float textWidth = ImGui::CalcTextSize("MyEngine").x;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::SetCursorPosY(windowHeight * 0.1f);

    ImGui::Text("MyEngine");
    ImGui::PopFont();

    // Buttons
    float btnGroupWidth = 410.0f;
    ImGui::SetCursorPosX((windowWidth - btnGroupWidth) * 0.5f);

    float btnWidth = 200.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float totalWidth = (btnWidth * 2) + spacing;

    ImGui::Dummy(ImVec2(0.0f, 40.0f));

    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    
    ImGui::SetCursorPosX((windowWidth - totalWidth) * 0.5f);
    ImGui::SetCursorPosY(windowHeight * 0.5f);
    
    // Button Style
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    
    if (ImGui::Button("Create New", ImVec2(btnWidth, 50)))
    {
        m_ShowNewProjectPopup = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Existing", ImVec2(btnWidth, 50)))
    {
        std::string pickedFolder = OpenFolderDialog();
        if (!pickedFolder.empty()) {
            
            std::filesystem::path p(pickedFolder);
            if (p.filename().empty()) {
                p = p.parent_path();
            }
            
            std::string projectName = p.filename().string();
            std::string projectFile = (p / (projectName + ".meproject")).string();
            
            Project::Load(projectFile);
            m_EngineContext->SetState(EngineState::Edit);
            m_EngineContext->OnEngineLoaded();
        }
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    
    // New Project Popup
    if (m_ShowNewProjectPopup) {
        ImGui::OpenPopup("New Project Setup");
    }

    if (ImGui::BeginPopupModal("New Project Setup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char nameBuf[128] = "MyNewGame";
        static char pathBuf[256] = "/Users/priyanshukaushik/MyEngineProjects";

        ImGui::InputText("Project Name", nameBuf, IM_ARRAYSIZE(nameBuf));
        
        ImGui::InputText("Location", pathBuf, IM_ARRAYSIZE(pathBuf));
        ImGui::SameLine();
        if(ImGui::Button("Browse")) {
            std::string pickedPath = OpenFolderDialog();
            if(!pickedPath.empty()) {
                strncpy(pathBuf, pickedPath.c_str(), sizeof(pathBuf));
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(120, 0))) {
            Project::New(pathBuf, nameBuf);
            m_EngineContext->SetState(EngineState::Edit);
            m_EngineContext->OnEngineLoaded();
            m_ShowNewProjectPopup = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_ShowNewProjectPopup = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

// FolderDialog Win/Mac
std::string EditorContext::OpenFolderDialog() {
#ifdef __APPLE__
    // macOS Logic (AppleScript)
    std::string cmd = "osascript -e 'POSIX path of (choose folder with prompt \"Select Project Location\")' 2>/dev/null";
    char buffer[1024];
    std::string result = "";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) result += buffer;
        pclose(pipe);
    }
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;

#elif _WIN32
    // --- Windows Logic (Win32 API) ---
    TCHAR szDir[MAX_PATH];
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = ("Select Project Location");
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != 0) {
        SHGetPathFromIDList(pidl, szDir);
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }
        return std::string(szDir);
    }
    return "";
#else
    return ""; // Linux etc. not handled yet
#endif
}
