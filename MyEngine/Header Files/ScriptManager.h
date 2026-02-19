//
//  ScriptManager.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 17/02/2026.
//

#pragma once
#include "sol/sol.hpp"
#include <unordered_map>

class Coordinator;

class ScriptManager {
public:
    void Init();
    
    sol::environment CreateEntityEnvironment(const std::string& scriptPath);
    
    sol::state& GetLuaState() { return m_Lua; }
    
    void SetCoordinator(Coordinator* aCoordinator);
private:
    Coordinator* m_Coordinator;
    sol::state m_Lua;
    std::unordered_map<std::string, sol::protected_function> m_CompiledScripts;
};
