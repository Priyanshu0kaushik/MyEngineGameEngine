//
//  ScriptSystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 17/02/2026.
//

#include "ECSSystems/ScriptSystem.h"
#include "ECS/Coordinator.h"
#include "EngineContext.h"

void ScriptSystem::Init()
{
    m_ScriptManager = std::make_unique<ScriptManager>();
    m_ScriptManager->SetCoordinator(m_Coordinator);
    m_ScriptManager->SetEngineContext(m_EngineContext);
    m_ScriptManager->Init();
}

void ScriptSystem::SetEngineContext(EngineContext *engineContext){
    m_EngineContext = engineContext;
}

void ScriptSystem::ReloadAllScripts()
{
    m_ScriptManager->ClearCache();

    for (auto const& entity : mEntities)
    {
        LoadScript(entity);
    }
    std::cout << "All Scripts Reloaded Successfully!" << std::endl;
}

void ScriptSystem::LoadScript(Entity entity)
{
    auto* script = m_Coordinator->GetComponent<ScriptComponent>(entity);
    script->env = m_ScriptManager->CreateEntityEnvironment(script->scriptPath);
    
    script->onCreate = script->env["OnCreate"];
    script->onUpdate = script->env["OnUpdate"];
    
    if (script->env["OnCreate"].valid()) {
        script->env["OnCreate"](entity);
    }
    script->initialized = true;
}

void ScriptSystem::Update(float deltaTime)
{
    for (auto const& entity : mEntities)
    {
        auto* script = m_Coordinator->GetComponent<ScriptComponent>(entity);
        
        if(!script || script->scriptPath.empty()) continue;
        
        if (!script->initialized)
        {
            script->env = m_ScriptManager->CreateEntityEnvironment(script->scriptPath);
            
            script->onCreate = script->env["OnCreate"];
            script->onUpdate = script->env["OnUpdate"];

            if (script->onCreate.valid()) {
                auto result = script->onCreate(entity);
                if (!result.valid()) {
                    sol::error err = result;
                    std::cerr << "Lua OnCreate Error: " << err.what() << std::endl;
                }
            }
            script->initialized = true;
        }

        // Script update
        if (script->onUpdate.valid()) {
            auto result = script->onUpdate(entity, deltaTime);
            
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "Lua OnUpdate Error (" << script->scriptPath << "): " << err.what() << std::endl;
            }
        }
    }
}
