//
//  ScriptManager.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 19/02/2026.
//

#include "ScriptManager.h"
#include "InputManager.h"
#include "ECS/Coordinator.h"
#include "Components.h"
#include <glm/glm.hpp>

void ScriptManager::Init()
{
    
    m_Lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);
    
    // Bindings
    
    m_Lua.set_function("IsKeyPressed", [](int keyCode) {
        return InputManager::Get().IsKeyPressed(keyCode);
    });
    
    m_Lua.new_enum("LightType",
        "Directional", LightType::Directional,
        "Point", LightType::Point,
        "Spot", LightType::Spot
    );

    m_Lua.new_enum("ColliderType",
        "None", ColliderType::None,
        "Box", ColliderType::Box,
        "Sphere", ColliderType::Sphere
    );

    m_Lua.new_usertype<glm::vec3>("vec3",
        sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z
    );

    m_Lua.new_usertype<TransformComponent>("Transform",
        "position", &TransformComponent::position,
        "rotation", &TransformComponent::rotation,
        "scale", &TransformComponent::scale,
        "isDirty", &TransformComponent::isDirty,
        "SetPosition", &TransformComponent::SetPosition,
        "SetRotation", &TransformComponent::SetRotation,
        "SetScale", &TransformComponent::SetScale
    );

    m_Lua.new_usertype<RigidBodyComponent>("RigidBody",
        "velocity", &RigidBodyComponent::velocity,
        "acceleration", &RigidBodyComponent::acceleration,
        "mass", &RigidBodyComponent::mass,
        "gravityScale", &RigidBodyComponent::gravityScale,
        "isStatic", &RigidBodyComponent::isStatic,
        "isKinematic", &RigidBodyComponent::isKinematic,
        "AddForce", &RigidBodyComponent::AddForce
    );

    m_Lua.new_usertype<CameraComponent>("Camera",
        "fov", &CameraComponent::Fov,
        "near", &CameraComponent::Near,
        "far", &CameraComponent::Far,
        "yaw", &CameraComponent::Yaw,
        "pitch", &CameraComponent::Pitch,
        "isPrimary", &CameraComponent::IsPrimary
    );

    m_Lua.new_usertype<NameComponent>("NameComponent", "name", &NameComponent::Name);
    
    m_Lua.set_function("GetName", [&](Entity e) {
        return m_Coordinator->GetComponent<NameComponent>(e)->Name;
    });
    
    m_Lua.set_function("GetTransform", [&](Entity entity) {
        return m_Coordinator->GetComponent<TransformComponent>(entity);
    });

    m_Lua.set_function("GetRigidBody", [&](Entity entity) {
        return m_Coordinator->GetComponent<RigidBodyComponent>(entity);
    });

    m_Lua.set_function("GetCamera", [&](Entity entity) {
        return m_Coordinator->GetComponent<CameraComponent>(entity);
    });
}

void ScriptManager::SetCoordinator(Coordinator *aCoordinator){
    m_Coordinator = aCoordinator;
}

sol::environment ScriptManager::CreateEntityEnvironment(const std::string &scriptPath)
{
    sol::environment env(m_Lua, sol::create, m_Lua.globals());

    
    if (m_CompiledScripts.find(scriptPath) == m_CompiledScripts.end()) {
        auto loadResult = m_Lua.load_file(scriptPath);
        if (!loadResult.valid()) {
            sol::error err = loadResult;
            std::cerr << "Failed to load script: " << scriptPath << "\n" << err.what() << std::endl;
            return env;
        }
        m_CompiledScripts[scriptPath] = loadResult;
    }

    sol::protected_function scriptFunc = m_CompiledScripts[scriptPath];
    sol::set_environment(env, scriptFunc);

    auto scriptResult = scriptFunc();
    
    if (!scriptResult.valid()) {
        sol::error err = scriptResult;
        std::cerr << "Script execution error: " << err.what() << std::endl;
    }

    return env;
}
