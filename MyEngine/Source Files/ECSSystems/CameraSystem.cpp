//
//  CameraSystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 07/12/2025.
//

#include "ECSSystems/CameraSystem.h"
#include "GLAD/include/glad/glad.h"
#include "ECS/Coordinator.h"
#include "glfw3.h"

void CameraSystem::Init()
{
    m_EditorCamera = m_Coordinator->CreateEntity();
    m_Coordinator->GetComponent<NameComponent>(m_EditorCamera)->Name = "Editor Camera";
    CameraComponent cam{};
    TransformComponent transform{};
    transform.position = {0,5,15};
    m_Coordinator->AddComponent(m_EditorCamera, transform);
    m_Coordinator->AddComponent(m_EditorCamera, cam);
    m_MainCam = m_EditorCamera;
    
    m_CurrentCameraSpeed = m_MinCameraSpeed;
}

void CameraSystem::Update()
{
    
}

void CameraSystem::OnPlayMode()
{
    for(auto Entity: mEntities)
    {
        CameraComponent* camComp = m_Coordinator->GetComponent<CameraComponent>(Entity);
        if(camComp->IsPrimary) m_MainCam = Entity;
    }
}

void CameraSystem::LookAt(const glm::vec3& target, const glm::vec3& up){
    if(m_MainCam==UINT32_MAX) return;
    CameraComponent* camera = m_Coordinator->GetComponent<CameraComponent>(m_MainCam);
    if(!camera) return;
    
    camera->CameraTarget = target;
    camera->Up = up;
}

glm::mat4 CameraSystem::GetView() const{
    if(m_MainCam==UINT32_MAX) return glm::mat4();
    TransformComponent* transform = m_Coordinator->GetComponent<TransformComponent>(m_MainCam);
    CameraComponent* camera = m_Coordinator->GetComponent<CameraComponent>(m_MainCam);
    if(!camera || !transform) return glm::mat4(1.0f);
    glm::vec3 forward = transform->GetForward();
    glm::vec3 up = transform->GetUp();
    
    return glm::lookAt(transform->position, transform->position + forward, up);
}

glm::mat4 CameraSystem::GetCameraProjection() const{
    if(m_MainCam==UINT32_MAX) return glm::mat4();
    CameraComponent* camera = m_Coordinator->GetComponent<CameraComponent>(m_MainCam);
    if(!camera) return glm::mat4();
    return glm::perspective(glm::radians(camera->Fov), camera->AspectRatio, camera->Near, camera->Far);
}

void CameraSystem::ProcessInput(GLFWwindow* aWindow, float aDeltaTime){
    if(m_MainCam==UINT32_MAX) return;
    ProcessMouseInput(aWindow, aDeltaTime);
    ProcessKeyboardInput(aWindow, aDeltaTime);
}


void CameraSystem::ProcessKeyboardInput(GLFWwindow* aWindow ,float aDeltaTime){
    TransformComponent* transform = m_Coordinator->GetComponent<TransformComponent>(m_MainCam);
    CameraComponent* camera = m_Coordinator->GetComponent<CameraComponent>(m_MainCam);
    if(!camera || !transform) return;
    
    const float cameraSpeed = m_CurrentCameraSpeed * aDeltaTime;
    glm::vec3 forward = transform->GetForward();
    glm::vec3 right = transform->GetRight();

    if (glfwGetKey(aWindow, GLFW_KEY_W) == GLFW_PRESS) transform->position += forward * cameraSpeed;
    if (glfwGetKey(aWindow, GLFW_KEY_S) == GLFW_PRESS) transform->position -= forward * cameraSpeed;
    if (glfwGetKey(aWindow, GLFW_KEY_A) == GLFW_PRESS) transform->position -= right * cameraSpeed;
    if (glfwGetKey(aWindow, GLFW_KEY_D) == GLFW_PRESS) transform->position += right * cameraSpeed;
    if (glfwGetKey(aWindow, GLFW_KEY_E) == GLFW_PRESS) transform->position += camera->Up * cameraSpeed;
    if (glfwGetKey(aWindow, GLFW_KEY_Q) == GLFW_PRESS) transform->position -= camera->Up * cameraSpeed;
}

void CameraSystem::ProcessMouseInput(GLFWwindow* aWindow, float aDeltaTime)
{
    TransformComponent* transform = m_Coordinator->GetComponent<TransformComponent>(m_MainCam);
    CameraComponent* camera = m_Coordinator->GetComponent<CameraComponent>(m_MainCam);
    if (!camera || !transform) return;
    
    static double lastX = 0.0, lastY = 0.0;

    double xpos, ypos;
    glfwGetCursorPos(aWindow, &xpos, &ypos);

    if (bFirstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        bFirstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos);
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    glm::vec3 rot = transform->rotation;
    rot.y += xoffset; // Yaw
    rot.x += yoffset; // Pitch

    if (rot.x > 89.0f) rot.x = 89.0f;
    if (rot.x < -89.0f) rot.x = -89.0f;

    transform->SetRotation(rot);
}

void CameraSystem::OnReleaseCamControl(){
    bFirstMouse = true;

}

void CameraSystem::ProcessMouseScroll(double yoffset)
{
    m_CurrentCameraSpeed -= yoffset * .2f;
    if (m_CurrentCameraSpeed < m_MinCameraSpeed) m_CurrentCameraSpeed = m_MinCameraSpeed;
    if (m_CurrentCameraSpeed > m_MaxCameraSpeed) m_CurrentCameraSpeed = m_MaxCameraSpeed;
}
