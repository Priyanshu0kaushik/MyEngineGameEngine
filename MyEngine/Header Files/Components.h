//
//  Components.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 19/11/2025.
//

#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "AssetData.h"


struct TransformComponent
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = glm::vec3(1.0f);
    
    static constexpr const char* TypeName = "Transform Component";
    static constexpr const bool UniquePerEntity = true;

};

struct NameComponent
{
    std::string Name = "GameObject";
    
    static constexpr const char* TypeName = "Name Component";
    static constexpr const bool UniquePerEntity = true;
};

struct MeshComponent
{
    Material material;
    uint32_t meshID = UINT32_MAX;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    int indexCount = 0;
    bool uploaded = false;
    
    static constexpr const char* TypeName = "Mesh Component";
    static constexpr const bool UniquePerEntity = true;
};


struct CameraComponent
{
    glm::vec3 CameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
    
    float Fov = 60.f;
    float Near = 0.1f;
    float Far = 100.f;

    float AspectRatio = 1280.f/720.f;
    float Yaw = -90.0f;
    float Pitch = 0.0f;
    
    bool IsPrimary = false;
    static constexpr const char* TypeName = "Camera Component";
    static constexpr const bool UniquePerEntity = true;
};

