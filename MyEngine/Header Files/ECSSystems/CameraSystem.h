//
//  CameraSystem.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 06/12/2025.
//

#pragma once
#include "ECS/ECSSystem.h"
#include "Components.h"
#include "ECS/Coordinator.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GLFWwindow;
class CameraSystem : public ECSSystem{
public:
    void Init() override;
    void Update();
    void SetCoordinator(Coordinator* aCoordinator){m_Coordinator = aCoordinator;}
    glm::mat4 GetCameraProjection() const;
    
    void LookAt(const glm::vec3& target, const glm::vec3& up);
    glm::mat4 GetView() const;
    
    void OnReleaseCamControl();
    void ProcessInput(GLFWwindow* aWindow, float aDeltaTime);
    void ProcessMouseScroll(double yoffset);
    
    void OnPlayMode();
private:
    void ProcessKeyboardInput(GLFWwindow* aWindow, float aDeltaTime);
    void ProcessMouseInput(GLFWwindow* aWindow, float aDeltaTime);
private:
    Coordinator* m_Coordinator;

    float m_CurrentCameraSpeed = 2.5f;
    float m_MaxCameraSpeed = 15.f;
    float m_MinCameraSpeed = 1.5f;
    bool bFirstMouse = true;

    Entity m_MainCam;
    Entity m_EditorCamera;

};
