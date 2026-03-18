//
//  EngineContext.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 03/11/2025.
//
#pragma once

#include "GLAD/include/glad/glad.h"
#include "glfw3.h"
#include "AssetData.h"
#include "ECS/Coordinator.h"
#include "MessageQueue.h"
#include "ECSSystems/CameraSystem.h"
#include <queue>
#include <memory>
#include <string>

class RenderSystem;
class LightSystem;
class PhysicsSystem;
class DebugGizmosSystem;
class ScriptSystem;
class TerrainSystem;
class UISystem;
class Scene;
class ShaderManager;
class Coordinator;
class MessageQueue;
class Message;
class EditorContext;


enum class EngineState {
    Edit = 0,
    Play = 1,
    Launcher = 2
};

class Scene;
class Shader;
class EditorContext;

class EngineContext{
public:
    EngineContext(int width, int height, const char* title);
    ~EngineContext();
    void OnEngineLoaded();
    
    Scene* GetScene(){return m_Scene;}
    GLFWwindow* GetWindow(){return m_Window;}
    Coordinator* GetCoordinator(){return m_Coordinator;}
    ShaderManager* GetShaderManager(){return m_ShaderManager;}
    std::shared_ptr<CameraSystem> GetCameraSystem(){ return cameraSystem;}
    unsigned int GetViewportTexture(){return m_ViewportTexture;}
    int GetFPS(){ return FPS;}
    
    void Draw();
    void Shutdown();
    
    void RequestSceneChange(std::string path, bool pathIsAbsolute = false);
    void LoadScene(std::string path);
    void SaveScene();
    
    void QuitGame();
    
    void OnStartControlCam();
    void OnReleaseCamControl();
    
    EngineState GetState() { return m_State; }
    void SetState(EngineState newState);
    
    void PushMessage(std::unique_ptr<Message> msg){
        m_MessageQueue->Push(std::move(msg));
    }
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
        EngineContext* engineContext = reinterpret_cast<EngineContext*>(glfwGetWindowUserPointer(window));
        if (engineContext && engineContext->bControllingCamera){
            engineContext->cameraSystem->ProcessMouseScroll(static_cast<float>(yoffset));
        }
    }
    
public:
    Entity CreateEntity(char* Name);
    void DeleteEntity(Entity aEntity);
    Entity DuplicateEntity(Entity sourceEntity);
private:
    void InitViewportFramebuffer(int width, int height);
    void InitShadowMap();
    void OnEditMode();
    void ProcessMessages();
    void SendMessageToAssetManager(std::unique_ptr<Message> msg);
    void InitWindow(int width, int height, const char* title);
    void Cleanup();
private:
    EngineState m_State = EngineState::Edit;

    Coordinator* m_Coordinator = nullptr;
    GLFWwindow* m_Window = nullptr;
    Scene* m_Scene = nullptr;
    ShaderManager* m_ShaderManager = nullptr;
    EditorContext* m_EditorContext = nullptr;
    
    std::shared_ptr<MessageQueue> m_MessageQueue;
    std::shared_ptr<RenderSystem> renderSystem;
    std::shared_ptr<CameraSystem> cameraSystem;
    std::shared_ptr<LightSystem> lightSystem;
    std::shared_ptr<PhysicsSystem> physicsSystem;
    std::shared_ptr<DebugGizmosSystem> debugSystem;
    std::shared_ptr<ScriptSystem> scriptSystem;
    std::shared_ptr<TerrainSystem> terrainSystem;
    std::shared_ptr<UISystem> uiSystem;
    
    std::string m_PendingScenePath = "";
    
    float m_ViewportWidth, m_ViewportHeight;
    unsigned int m_ViewportFBO, m_ViewportTexture, m_ViewportRBO;
    
    unsigned int m_DepthMapFBO, m_DepthMapTexture;
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    
    unsigned int m_gBuffer, m_gDepthRBO;
    unsigned int m_gPosition, m_gNormal, m_gAlbedoSpec;
    
    float m_DeltaTime = 0.0f;
    float m_LastFrameTime = 0.0f;
    float fpsTimer = 0.0f;
    int frameCount = 0;
    int FPS = 0;
    
    bool m_SceneChangeRequested = false;
    bool bControllingCamera = false;
    
};
