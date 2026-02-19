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
#include "AssetManager.h"
#include "sol/sol.hpp"


struct TransformComponent
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = glm::vec3(1.0f);
    
    bool isDirty = true;
    
    void SetPosition(const glm::vec3& newPos) {
        if(position == newPos) return;
        position = newPos;
        isDirty = true;
    }

    void SetRotation(const glm::vec3& newRot) {
        if(rotation == newRot) return;
        rotation = newRot;
        isDirty = true;
    }

    void SetScale(const glm::vec3& newScale) {
        if(scale == newScale) return;
        scale = newScale;
        isDirty = true;
    }
    
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

    std::string meshPath  = "";

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

enum class LightType {
    Directional,
    Point,
    Spot
};

struct LightComponent {
    LightType type = LightType::Point;
    
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    
    static constexpr const char* TypeName = "Light Component";
    static constexpr const bool UniquePerEntity = true;
};

struct RigidBodyComponent {
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    float mass = 1.0f;
    float gravityScale = 1.0f;
    bool isStatic = false;
    bool isKinematic = false;
    
    void AddForce(glm::vec3 force)
    {
        if (!isStatic) {
            acceleration += force / mass;
        }
    }
    
    static constexpr const char* TypeName = "RigidBody Component";
    static constexpr const bool UniquePerEntity = true;
};

enum class ColliderType{
    None = 0,
    Box = 1,
    Sphere = 2
};

struct ColliderComponent {
    
    ColliderType type = ColliderType::None;

    glm::mat4 worldTransform = glm::mat4(1.0f);
    glm::mat4 worldInverse = glm::mat4(1.0f);
    
    float bounciness = 0.2f;
    float friction = 0.5f;

    glm::vec3 worldMin;
    glm::vec3 worldMax;

    glm::vec3 center = {0, 0, 0};
    
    bool isTrigger = false;
    bool isColliding = false;
    bool isDirty = false;
    
    static constexpr const char* TypeName = "Collider Component";
    static constexpr const bool UniquePerEntity = true;
};

struct BoxColliderComponent
{
    glm::vec3 extents = {0.5f, 0.5f, 0.5f};
    
    static constexpr const char* TypeName = "Box Collider";
    static constexpr const bool UniquePerEntity = true;
};

struct SphereColliderComponent
{
    float radius = 0.5f;

    static constexpr const char* TypeName = "Sphere Collider";
    static constexpr const bool UniquePerEntity = true;
};

struct ScriptComponent
{
    std::string scriptPath;
    sol::environment env;
    bool initialized = false;
    
    sol::protected_function onUpdate;
    sol::protected_function onCreate;
    
    static constexpr const char* TypeName = "Script Component";
    static constexpr const bool UniquePerEntity = true;
};
