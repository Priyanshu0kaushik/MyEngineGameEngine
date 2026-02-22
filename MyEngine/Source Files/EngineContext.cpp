//
//  EngineContext.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 03/11/2025.
//

#include "EngineContext.h"
#include "Scene.h"
#include "EditorContext.h"
#include "JobSystem.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "Project.h"

/**
 * @brief Initializes the Core Engine Loop.
 * Sets up the Entity Component System (ECS), Windowing (GLFW), and rendering pipelines.
 */
EngineContext::EngineContext(int width, int height, const char* title)
{
    // 1. ECS + Thread Initialization
    m_Coordinator = new Coordinator();
    m_Coordinator->Init();
    JobSystem::Get().Init();
    
    if (!glfwInit()) throw std::runtime_error("Failed to init GLFW");
    InitWindow(width, height, title);

    // 2. ImGui Initializations
    m_EditorContext = new EditorContext();
    m_EditorContext->Init(m_Window, this);

    // 3. Register Basic Components
    m_Coordinator->RegisterComponent<NameComponent>();
    m_Coordinator->RegisterComponent<TransformComponent>();
    m_Coordinator->RegisterComponent<MeshComponent>();
    m_Coordinator->RegisterComponent<CameraComponent>();
    m_Coordinator->RegisterComponent<LightComponent>();
    m_Coordinator->RegisterComponent<RigidBodyComponent>();
    m_Coordinator->RegisterComponent<ColliderComponent>();
    m_Coordinator->RegisterComponent<BoxColliderComponent>();
    m_Coordinator->RegisterComponent<SphereColliderComponent>();
    m_Coordinator->RegisterComponent<ScriptComponent>();
    m_Coordinator->RegisterComponent<TerrainComponent>();
    

    SetState(EngineState::Launcher);
}

void EngineContext::OnEngineLoaded()
{
    // 1. Messaging & Assets Initializations
    m_MessageQueue = std::make_shared<MessageQueue>();
    AssetManager::Allocate();
    AssetManager::Get().SetMessageQueue(m_MessageQueue);

    // 2. Systems Setup
    renderSystem = m_Coordinator->RegisterSystem<RenderSystem>();
    cameraSystem = m_Coordinator->RegisterSystem<CameraSystem>();
    lightSystem = m_Coordinator->RegisterSystem<LightSystem>();
    physicsSystem = m_Coordinator->RegisterSystem<PhysicsSystem>();
    debugSystem = m_Coordinator->RegisterSystem<DebugGizmosSystem>();
    scriptSystem = m_Coordinator->RegisterSystem<ScriptSystem>();
    terrainSystem = m_Coordinator->RegisterSystem<TerrainSystem>();
    


    // Component Signatures
    Signature RenderSignature;
    RenderSignature.set(m_Coordinator->GetComponentType<TransformComponent>());
    RenderSignature.set(m_Coordinator->GetComponentType<MeshComponent>());
    m_Coordinator->SetSystemSignature<RenderSystem>(RenderSignature);
    renderSystem->SetCoordinator(m_Coordinator);
    
    
    Signature CameraSignature;
    CameraSignature.set(m_Coordinator->GetComponentType<TransformComponent>());
    CameraSignature.set(m_Coordinator->GetComponentType<CameraComponent>());
    m_Coordinator->SetSystemSignature<CameraSystem>(CameraSignature);
    cameraSystem->SetCoordinator(m_Coordinator);

 
    Signature lightSignature;
    lightSignature.set(m_Coordinator->GetComponentType<TransformComponent>());
    lightSignature.set(m_Coordinator->GetComponentType<LightComponent>());
    m_Coordinator->SetSystemSignature<LightSystem>(lightSignature);
    lightSystem->SetCoordinator(m_Coordinator);
    
    Signature physicsSignature;
    physicsSignature.set(m_Coordinator->GetComponentType<TransformComponent>());
    physicsSignature.set(m_Coordinator->GetComponentType<RigidBodyComponent>());
    m_Coordinator->SetSystemSignature<PhysicsSystem>(physicsSignature);
    physicsSystem->SetCoordinator(m_Coordinator);
    
    Signature debugSignature;
    debugSignature.set(m_Coordinator->GetComponentType<TransformComponent>());
    debugSignature.set(m_Coordinator->GetComponentType<ColliderComponent>());
    m_Coordinator->SetSystemSignature<DebugGizmosSystem>(debugSignature);
    debugSystem->SetCoordinator(m_Coordinator);
    
    Signature scriptSignature;
    scriptSignature.set(m_Coordinator->GetComponentType<ScriptComponent>());
    m_Coordinator->SetSystemSignature<ScriptSystem>(scriptSignature);
    scriptSystem->SetCoordinator(m_Coordinator);
    
    Signature terrainSignature;
    terrainSignature.set(m_Coordinator->GetComponentType<TerrainComponent>());
    terrainSignature.set(m_Coordinator->GetComponentType<TransformComponent>());
    m_Coordinator->SetSystemSignature<TerrainSystem>(terrainSignature);
    terrainSystem->SetCoordinator(m_Coordinator);
    
    renderSystem->Init();
    lightSystem->Init();
    physicsSystem->Init();
    debugSystem->Init();
    scriptSystem->Init();
    
    physicsSystem->SetTerrainSystem(terrainSystem);

    // 3. Setup Scene & Rendering Context
    m_Scene = new Scene(*m_Coordinator, renderSystem, cameraSystem);
    m_ShaderManager = new ShaderManager();
    m_ShaderManager->Init();

    InitViewportFramebuffer(2048, 2048);
    InitShadowMap();

    // 4. Load the actual scene data
    OnEditMode();
    
    m_EditorContext->OnEditorLaunched();
    
    SetState(EngineState::Edit);
}

/**
 * @brief Configures the Shadow Map Framebuffer (FBO).
 */
void EngineContext::InitShadowMap() {
    glGenFramebuffers(1, &m_DepthMapFBO);

    // Create Depth Texture
    glGenTextures(1, &m_DepthMapTexture);
    glBindTexture(GL_TEXTURE_2D, m_DepthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    
    // Set texture parameters for shadow sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach texture to Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthMapTexture, 0);
    
    // Not rendering color data
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief Initializes the Off-Screen Framebuffer.
 * Instead of rendering directly to the window, we render to this texture (`m_ViewportTexture`).
 * This texture is then passed to ImGui to be displayed as an Image inside an Editor Window.
 */
void EngineContext::InitViewportFramebuffer(int width, int height){
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    
    glGenFramebuffers(1, &m_ViewportFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);

    // Create Color Attachment
    glGenTextures(1, &m_ViewportTexture);
    glBindTexture(GL_TEXTURE_2D, m_ViewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ViewportTexture, 0);

    // Create Depth/Stencil Attachment (RBO) for depth testing
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

    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, GL_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GL_TRUE);

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
    
    glfwSetKeyCallback(m_Window, [](GLFWwindow* m_Window, int key, int scancode, int action, int mods) {
        InputManager::Get().UpdateKeyState(key, action);
    });
    
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    glEnable(GL_DEPTH_TEST);
}

void EngineContext::OnStartControlCam(){
    bControllingCamera = true;
}

void EngineContext::OnReleaseCamControl(){
    bControllingCamera = false;
    cameraSystem->OnReleaseCamControl();
}

/**
 * @brief The Main Engine Loop.
 * 1. Calculates Delta Time.
 * 2. PASS 1: Renders the Shadow Map.
 * 3. PASS 2: Renders the Main Scene to Off-screen FBO.
 * 4. PASS 3: Renders the Editor UI (ImGui) to the Window.
 */
void EngineContext::Draw(){
    
    while (!glfwWindowShouldClose(m_Window))
    {
        // Calculate Delta Time (Time between frames)
        float currentFrame = glfwGetTime();
        m_DeltaTime = currentFrame - m_LastFrameTime;
        m_LastFrameTime = currentFrame;
        
        fpsTimer += m_DeltaTime;
        frameCount++;

        // updating FPS in every 0.5 second
        if (fpsTimer >= 0.5f) {
            FPS = (int)(frameCount / fpsTimer);
            frameCount = 0;
            fpsTimer = 0.0f;
        }
        // Prepare ImGui frame
        m_EditorContext->BeginFrame();
        
        if (m_State == EngineState::Launcher) {
            // Launcher UI
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            m_EditorContext->RenderLauncher();
        }
        else
        {
            // Normal Editor Logic (After project is loaded)
            ProcessMessages();
            m_Scene->SyncLoadedAssets();
            cameraSystem->Update();

            // PASS 1: Shadow Mapping
            // Render the scene from the Light's perspective into the Depth Buffer

            Shader* shadowShader = m_ShaderManager->Get("ShadowMap");
            if (shadowShader) {
                shadowShader->Use();
                glm::mat4 lightSpaceMatrix = lightSystem->GetLightSpaceMatrix();
                shadowShader->SetMatrix4(lightSpaceMatrix, "shadowMapMatrix");

                glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
                glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
                glClear(GL_DEPTH_BUFFER_BIT); // Only clearing depth, no color

                renderSystem->Render(*shadowShader);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
            
            // PASS 2: Main Scene Rendering
            // Render the scene to the custom Framebuffer (m_ViewportFBO)

            glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
            
            glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
            glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            Shader* mainShader = m_ShaderManager->Get("Main");
            if(mainShader){
                mainShader->Use();
                
                // Upload Camera Matrices
                mainShader->SetMatrix4(cameraSystem->GetView(), "viewMatrix");
                mainShader->SetMatrix4(cameraSystem->GetCameraProjection(), "projectionMatrix");

                // Upload Shadow Matrix & Map
                glm::mat4 lightSpaceMatrix = lightSystem->GetLightSpaceMatrix();
                mainShader->SetMatrix4(lightSpaceMatrix, "shadowMapMatrix");
                
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, m_DepthMapTexture);
                mainShader->SetInt("shadowMap", 3);
                
                // Draw Scene
                terrainSystem->Render(*mainShader);
                renderSystem->Render(*mainShader);
                lightSystem->Render(*mainShader);
            }

            if(bControllingCamera) cameraSystem->ProcessInput(m_Window, m_DeltaTime);
            
            // Collision Debug Update In Edit
            if(m_State == EngineState::Edit){
                if(m_EditorContext->GetSelectedEntity() != UINT32_MAX){
                    physicsSystem->UpdateBounds(m_EditorContext->GetSelectedEntity());
                    debugSystem->Update(m_EditorContext->GetSelectedEntity());
                }
                Shader* debugShader = m_ShaderManager->Get("DebugShader");
                if(debugShader)
                {
                    debugShader->Use();
                    debugShader->SetMatrix4(cameraSystem->GetView(), "view");
                    debugShader->SetMatrix4(cameraSystem->GetCameraProjection(), "projection");
                    
                    glDisable(GL_DEPTH_TEST);
                    debugSystem->Render();
                    glEnable(GL_DEPTH_TEST);
                }
            }
            
            // Physics and Script Update In Play
            if(m_State == EngineState::Play){
                physicsSystem->Update(m_DeltaTime);
                scriptSystem->Update(m_DeltaTime);
            }
            
            //Unbinding
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            // Editor UI
            m_EditorContext->RenderEditor();
        }
        
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}

void EngineContext::Cleanup(){
    delete m_Scene;
    delete m_ShaderManager;
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
    }
}

void EngineContext::SetState(EngineState newState)
{
    // 1. Saving Scene before playing
    if(m_State == EngineState::Edit && newState == EngineState::Play)
    {
        std::cout << "Saving scene before play..." << std::endl;
        GetScene()->Save();
    }

    // 2. Returning to EDIT mode: Reset everything by loading the saved state
    if(m_State == EngineState::Play && newState == EngineState::Edit)
    {
        std::cout << "Resetting scene after play..." << std::endl;
        OnEditMode();
    }
    m_State = newState;
}

void EngineContext::OnEditMode()
{
    std::string startScenePath = Project::GetActiveAbsoluteScenePath();
    if (!startScenePath.empty()) {
        m_Scene->Load(startScenePath);
        cameraSystem->Init();

    }
}

void EngineContext::Shutdown(){
    m_EditorContext->EndFrame();
    AssetManager::Get().CleanUp();
    AssetManager::DeAllocate();
    JobSystem::Get().Shutdown();
    Cleanup();
    glfwTerminate();
}
