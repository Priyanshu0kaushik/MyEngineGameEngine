//
//  ScriptManager.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 19/02/2026.
//

#include "ScriptManager.h"
#include "EngineContext.h"
#include "Scene.h"
#include "InputManager.h"
#include "Project.h"
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>

void ScriptManager::Init()
{
    
    m_Lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);
    
    // Bindings
    
    m_Lua.set_function("IsKeyPressed", [](int keyCode) {
        return InputManager::Get().IsKeyPressed(keyCode);
    });
    
    m_Lua.set_function("LoadScene", [this](std::string scenePath) {
        m_EngineContext->RequestSceneChange(scenePath);
    });
    
    m_Lua.set_function("QuitGame", [this]() {
        m_EngineContext->QuitGame();
    });
    
    m_Lua.set_function("StartPlayTimer", [this]() {
        m_EngineContext->StartPlayTimer();
    });
    
    m_Lua.set_function("StopPlayTimer", [this]() {
        return m_EngineContext->StopPlayTimer();
    });
    
    m_Lua.set_function("GetPlayTime", [this]() {
        return m_EngineContext->GetPlayTime();
    });
    
    m_Lua.set_function("FindEntityByName", [this](std::string Name){
        return m_EngineContext->GetScene()->FindEntityByName(Name);
    });
    
    m_Lua.set_function("DestroyEntity", [this](Entity entity){
        return m_EngineContext->DeleteEntity(entity);
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
    
    m_Lua.new_usertype<ColliderComponent>("Collider",
        "isColliding", &ColliderComponent::isColliding,
        "isTrigger", &ColliderComponent::isTrigger,
        "bounciness", &ColliderComponent::bounciness
    );

    m_Lua.new_usertype<glm::vec3>("vec3",
        sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,
        // Addition
        sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
        // Subtraction
        sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },
        // Multiplication (vec3 * float)
        sol::meta_function::multiplication, [](const glm::vec3& a, float b) { return a * b; },
        // Unary Minus (-vec3)
        sol::meta_function::unary_minus, [](const glm::vec3& a) { return -a; }
    );

    m_Lua.new_usertype<TransformComponent>("Transform",
        "position", &TransformComponent::position,
        "rotation", &TransformComponent::rotation,
        "scale", &TransformComponent::scale,
        "isDirty", &TransformComponent::isDirty,
        "GetForward", &TransformComponent::GetForward,
        "GetRight", &TransformComponent::GetRight,
        "GetUp", &TransformComponent::GetUp,
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
    
    m_Lua.new_usertype<UITextComponent>("UITextComponent",
        "text", &UITextComponent::text,
        "color", &UITextComponent::color
    );
    
    m_Lua.new_usertype<UIBaseComponent>("UIBase",
        "position", &UIBaseComponent::position,
        "zOrder", &UIBaseComponent::zOrder,
        "isVisible", &UIBaseComponent::isVisible  
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
    
    m_Lua.set_function("GetUIBase", [&](Entity entity) {
        return m_Coordinator->GetComponent<UIBaseComponent>(entity);
    });
    
    m_Lua.set_function("GetTextUI", [&](Entity entity) {
        return m_Coordinator->GetComponent<UITextComponent>(entity);
    });
    
    m_Lua.set_function("GetCollider", [&](Entity entity) {
        return m_Coordinator->GetComponent<ColliderComponent>(entity);
    });
    
    m_Lua.set_function("ReadFile", [](std::string path) {
        std::ifstream file(Project::GetAbsolutePath(path));
        if (!file.is_open()) return std::string("");
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    });

    m_Lua.set_function("AppendFile", [](std::string path, std::string content) {
        std::ofstream file(Project::GetAbsolutePath(path), std::ios_base::app);
        if (file.is_open()) {
            file << content;
            file.close();
        }
    });
    
    m_Lua.set_function("GetScript", [this](Entity entity) -> sol::table {
        auto* script = m_Coordinator->GetComponent<ScriptComponent>(entity);
        if(!script) return sol::nil;
        return script->env;
    });
}

void ScriptManager::SetCoordinator(Coordinator *aCoordinator){
    m_Coordinator = aCoordinator;
}

void ScriptManager::SetEngineContext(EngineContext *aEngineContext){
    m_EngineContext = aEngineContext;
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


//sol::environment ScriptManager::CreateEntityEnvironment(const std::string &scriptPath)
//{
//    // Isolated env — no global inheritance
//    sol::environment env(m_Lua, sol::create);
//
//    // Copy all engine-registered globals into this env
//    // Add any name here you want scripts to access
//    static const std::vector<std::string> k_EngineGlobals = {
//        // stdlib (you opened these in Init)
//        "print", "ipairs", "pairs", "next", "type", "tostring", "tonumber",
//        "math", "table", "string", "select", "unpack", "error", "assert",
//        // engine functions
//        "IsKeyPressed", "LoadScene", "QuitGame",
//        "StartPlayTimer", "StopPlayTimer", "GetPlayTime",
//        "FindEntityByName", "DestroyEntity", "GetScript",
//        "GetName", "GetTransform", "GetRigidBody", "GetCamera",
//        "GetUIBase", "GetTextUI", "ReadFile", "AppendFile",
//        // enums & usertypes (sol registers these as globals too)
//        "LightType", "ColliderType", "vec3",
//        "Transform", "RigidBody", "UITextComponent",
//        "UIBase", "Camera", "NameComponent"
//    };
//
//    for (const auto& name : k_EngineGlobals) {
//        env[name] = m_Lua[name];
//    }
//
//    // Load + cache script
//    if (m_CompiledScripts.find(scriptPath) == m_CompiledScripts.end()) {
//        auto loadResult = m_Lua.load_file(scriptPath);
//        if (!loadResult.valid()) {
//            sol::error err = loadResult;
//            std::cerr << "Failed to load script: " << scriptPath << "\n" << err.what();
//            return env;
//        }
//        m_CompiledScripts[scriptPath] = loadResult;
//    }
//
//    sol::protected_function scriptFunc = m_CompiledScripts[scriptPath];
//    sol::set_environment(env, scriptFunc);
//
//    auto result = scriptFunc();
//    if (!result.valid()) {
//        sol::error err = result;
//        std::cerr << "Script execution error: " << err.what();
//    }
//
//    // Inject InspectorVariables as per-env locals
//    sol::optional<sol::table> vars = env["InspectorVariables"];
//    if (vars.has_value()) {
//        for (auto& [_, entry] : vars.value()) {
//            if (!entry.is<sol::table>()) continue;
//            sol::table t = entry.as<sol::table>();
//
//            sol::optional<std::string> name = t["name"];
//            sol::optional<std::string> type = t["type"];
//            sol::object value = t["value"];
//
//            if (!name.has_value() || !type.has_value()) continue;
//
//            if (type.value() == "string") {
//                env[name.value()] = value.as<std::string>();
//            } else if (type.value() == "number") {
//                env[name.value()] = value.as<float>();
//            } else if (type.value() == "bool") {
//                env[name.value()] = value.as<bool>();
//            }
//        }
//    }
//
//    return env;
//}
