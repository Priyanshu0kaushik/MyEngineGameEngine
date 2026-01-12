//
//  EngineContext.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 03/11/2025.
//

#include "EngineContext.h"
#include "Scene.h"
#include "EditorContext.h"
#include "AssetManager.h"
#include "Project.h"




EngineContext::EngineContext(int width, int height, const char* title)
{
    m_Coordinator = new Coordinator();
    m_Coordinator->Init();
    
    
    if (!glfwInit())
        throw std::runtime_error("Failed to init GLFW");

    //GLFW Window
    InitWindow(width, height, title);
    InitViewportFramebuffer(500,500);
    //ImGUI

    TestProjectSetupInit();
    // Components
    m_Coordinator->RegisterComponent<TransformComponent>();
    m_Coordinator->RegisterComponent<MeshComponent>();
    m_Coordinator->RegisterComponent<CameraComponent>();
    
    // Systems
    renderSystem = m_Coordinator->RegisterSystem<RenderSystem>();
    renderSystem->SetCoordinator(m_Coordinator);
    Signature RenderSignature;
    RenderSignature.set(m_Coordinator->GetComponentType<TransformComponent>());
    RenderSignature.set(m_Coordinator->GetComponentType<MeshComponent>());
    m_Coordinator->SetSystemSignature<RenderSystem>(RenderSignature);
    
    cameraSystem = m_Coordinator->RegisterSystem<CameraSystem>();
    cameraSystem->SetCoordinator(m_Coordinator);
    Signature CameraSignature;
    CameraSignature.set(m_Coordinator->GetComponentType<TransformComponent>());
    CameraSignature.set(m_Coordinator->GetComponentType<CameraComponent>());
    m_Coordinator->SetSystemSignature<CameraSystem>(CameraSignature);
    
    renderSystem->Init();
    cameraSystem->Init();
    
    m_Scene = new Scene(*m_Coordinator, renderSystem, cameraSystem);

    m_MessageQueue = std::make_shared<MessageQueue>();
    
    AssetManager::Allocate();
    AssetManager::Get().SetMessageQueue(m_MessageQueue);

    m_EditorContext = new EditorContext();
    m_EditorContext->Init(m_Window, this);

}

void EngineContext::InitViewportFramebuffer(int width, int height){
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    
    glGenFramebuffers(1, &m_ViewportFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);

    glGenTextures(1, &m_ViewportTexture);
    glBindTexture(GL_TEXTURE_2D, m_ViewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ViewportTexture, 0);

    glGenRenderbuffers(1, &m_ViewportRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_ViewportRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Incomplete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void EngineContext::InitWindow(int width, int height, const char* title){
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    m_Window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!m_Window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetScrollCallback(m_Window, EngineContext::ScrollCallback);
    
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    glEnable(GL_DEPTH_TEST);

}

void EngineContext::TestProjectSetupInit(){
    std::string workspacePath = "/Users/priyanshukaushik/MyEngineProjects";
    std::string projectName = "TestProject";
    
    std::filesystem::path projectFile = std::filesystem::path(workspacePath) / projectName / (projectName + ".meproject");

    if (std::filesystem::exists(projectFile))
    {
        std::cout << "Loading existing project..." << std::endl;
        Project::Load(projectFile);
    }
    else
    {
        std::cout << "Creating new project..." << std::endl;
        Project::New(workspacePath, projectName);
    }
}

void EngineContext::OnStartControlCam(){
    bControllingCamera = true;
}

void EngineContext::OnReleaseCamControl(){
    bControllingCamera = false;
    cameraSystem->OnReleaseCamControl();
}


void EngineContext::Draw(){
    while (!glfwWindowShouldClose(m_Window))
    {
        float currentFrame = glfwGetTime();
        m_DeltaTime = currentFrame - m_LastFrameTime;
        m_LastFrameTime = currentFrame;
        
        m_EditorContext->BeginFrame();

        /* Render here */
        glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
        glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
        ProcessMessages();
        cameraSystem->Update();
        if(m_Shader) renderSystem->Render(*m_Shader);

        if(bControllingCamera) cameraSystem->ProcessInput(m_Window, m_DeltaTime);
        
        if(m_Shader && cameraSystem){
            m_Shader->Use();
            m_Shader->SetMatrix4(cameraSystem->GetView(), "viewMatrix");
            m_Shader->SetMatrix4(cameraSystem->GetCameraProjection(), "projectionMatrix");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // ImGUI
        m_EditorContext->Render();
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}

void EngineContext::Cleanup(){
    delete m_Scene;
    delete m_EditorContext;
    delete m_Coordinator;
}

Entity EngineContext::CreateEntity(char *Name){
    if(!m_Scene) return UINT32_MAX;
    return m_Scene->AddEntity(Name);
}

void EngineContext::DeleteEntity(Entity aEntity){
    if(m_Scene) m_Scene->RemoveEntity(aEntity);
}

void EngineContext::SendMessage(std::unique_ptr<Message> msg)
{
    AssetManager::Get().ProcessMessage(msg.get());
}

void EngineContext::ProcessMessages(){
    auto msg = m_MessageQueue->Pop();
    while(msg!=nullptr){
        SendMessage(std::move(msg));
        msg = m_MessageQueue->Pop();
    }
}

void EngineContext::Shutdown(){
    m_EditorContext->EndFrame();
    AssetManager::Get().CleanUp();
    AssetManager::DeAllocate();
    Cleanup();
    glfwTerminate();
}
