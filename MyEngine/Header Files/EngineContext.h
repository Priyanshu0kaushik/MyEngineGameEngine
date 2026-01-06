//
//  EngineContext.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 03/11/2025.
//
#pragma once

#include "glad.h"
#include "glfw3.h"
#include "RenderSystem.h"
#include "CameraSystem.h"
#include "AssetData.h"
#include "ECS/Coordinator.h"
#include "MessageQueue.h"

#include <queue>
#include <memory>

class Scene;
class Shader;
class EditorContext;

class EngineContext{
public:
    EngineContext(int width, int height, const char* title);
    void SetShader(Shader* aShader){ m_Shader = aShader;}
    Scene* GetScene(){return m_Scene;}
    GLFWwindow* GetWindow(){return m_Window;}
    Shader* GetShader(){return m_Shader;}
    Coordinator* GetCoordinator(){return m_Coordinator;}
    void InitViewportFramebuffer(int width, int height);
    void Draw();
    void Shutdown();
    unsigned int GetViewportTexture(){return m_ViewportTexture;}
    int GetFPS(){ return (int)(1/m_DeltaTime);}
    void OnStartControlCam();
    void OnReleaseCamControl();
    
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
private:
    void TestProjectSetupInit();
    void ProcessMessages();
    void SendMessage(std::unique_ptr<Message> msg);
    void InitWindow(int width, int height, const char* title);
    void Cleanup();
private:
    Coordinator* m_Coordinator = nullptr;
    GLFWwindow* m_Window = nullptr;
    Scene* m_Scene = nullptr;
    Shader* m_Shader = nullptr;
    EditorContext* m_EditorContext = nullptr;
    
    std::shared_ptr<MessageQueue> m_MessageQueue;
    std::shared_ptr<RenderSystem> renderSystem;
    std::shared_ptr<CameraSystem> cameraSystem;
    
    
    float m_ViewportWidth, m_ViewportHeight;
    unsigned int m_ViewportFBO, m_ViewportTexture, m_ViewportRBO;
    bool bControllingCamera = false;
    
    float m_DeltaTime = 0.0f;
    float m_LastFrameTime = 0.0f;
};
