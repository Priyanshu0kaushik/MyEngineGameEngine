//
//  CameraSystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 07/12/2025.
//

#include "ECSSystems/CameraSystem.h"
#include "GLAD/include/glad/glad.h"
#include "glfw3.h"

void CameraSystem::Init()
{
    m_EditorCamera = m_Coordinator->CreateEntity();
    m_Coordinator->GetComponent<NameComponent>(m_EditorCamera)->Name = "Editor Camera";
    CameraComponent cam{};
    TransformComponent transform{};
    transform.position = {0,5,-15};
    m_Coordinator->AddComponent(m_EditorCamera, transform);
    m_Coordinator->AddComponent(m_EditorCamera, cam);
    m_MainCam = m_EditorCamera;
}

void CameraSystem::Update()
{
    OnPlayMode();
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

    camera->CameraTarget = target;
    camera->Up = up;
}

glm::vec3 CameraSystem::GetCurrentCameraPosition() const
{
    if(m_MainCam==UINT32_MAX) return glm::vec3();
    TransformComponent* transform = m_Coordinator->GetComponent<TransformComponent>(m_MainCam);
    return transform->position;
}


glm::mat4 CameraSystem::GetView() const{
    if(m_MainCam==UINT32_MAX) return glm::mat4();
    TransformComponent* transform = m_Coordinator->GetComponent<TransformComponent>(m_MainCam);
    CameraComponent* camera = m_Coordinator->GetComponent<CameraComponent>(m_MainCam);
    return glm::lookAt(transform->position, transform->position + camera->Front, camera->Up);
}

glm::mat4 CameraSystem::GetCameraProjection() const{
    if(m_MainCam==UINT32_MAX) return glm::mat4();
    CameraComponent* camera = m_Coordinator->GetComponent<CameraComponent>(m_MainCam);
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

    const float cameraSpeed = m_CameraSpeedVar * aDeltaTime;
    if (glfwGetKey(aWindow, GLFW_KEY_W) == GLFW_PRESS)
        transform->position += cameraSpeed * camera->Front;
    if (glfwGetKey(aWindow, GLFW_KEY_S) == GLFW_PRESS)
        transform->position -= cameraSpeed * camera->Front;
    if (glfwGetKey(aWindow, GLFW_KEY_A) == GLFW_PRESS)
        transform->position -= glm::normalize(glm::cross(camera->Front, camera->Up)) * cameraSpeed;
    if (glfwGetKey(aWindow, GLFW_KEY_D) == GLFW_PRESS)
        transform->position += glm::normalize(glm::cross(camera->Front, camera->Up)) * cameraSpeed;
    if (glfwGetKey(aWindow, GLFW_KEY_E) == GLFW_PRESS)
        transform->position += camera->Up * cameraSpeed;
    if (glfwGetKey(aWindow, GLFW_KEY_Q) == GLFW_PRESS)
        transform->position -= camera->Up * cameraSpeed;
}

void CameraSystem::ProcessMouseInput(GLFWwindow* aWindow, float aDeltaTime)
{
    CameraComponent* camera = m_Coordinator->GetComponent<CameraComponent>(m_MainCam);

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

    camera->Yaw   += xoffset;
    camera->Pitch += yoffset;

    if (camera->Pitch > 89.0f) camera->Pitch = 89.0f;
    if (camera->Pitch < -89.0f) camera->Pitch = -89.0f;

    camera->Front.x = cos(glm::radians(camera->Yaw)) * cos(glm::radians(camera->Pitch));
    camera->Front.y = sin(glm::radians(camera->Pitch));
    camera->Front.z = sin(glm::radians(camera->Yaw)) * cos(glm::radians(camera->Pitch));
    camera->Front = glm::normalize(camera->Front);
}

void CameraSystem::OnReleaseCamControl(){
    bFirstMouse = true;

}

void CameraSystem::ProcessMouseScroll(double yoffset){
    if(m_MainCam==UINT32_MAX) return;
    CameraComponent* camera = m_Coordinator->GetComponent<CameraComponent>(m_MainCam);
    camera->Fov -= yoffset;
    if (camera->Fov < 1.0f) camera->Fov = 1.0f;
    if (camera->Fov > 90.0f) camera->Fov = 90.0f;
}
