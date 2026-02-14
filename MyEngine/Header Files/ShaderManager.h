//
//  ShaderManager.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 24/01/2026.
//

#pragma once
#include <unordered_map>
#include <memory>
#include "Shader.h"

class ShaderManager {
public:
    void Init() {
        AddShader("Main", "Shaders/VertexShader.glsl", "Shaders/FragmentShader.glsl");
        AddShader("ShadowMap", "Shaders/ShadowDepthVertexShader.glsl", "Shaders/ShadowDepthFragmentShader.glsl");
        AddShader("DebugShader", "Shaders/DebugGizmosVertexShader.glsl", "Shaders/DebugGizmosFragmentShader.glsl");
    }

    void AddShader(const std::string& name, const char* vPath, const char* fPath) {
        m_Shaders[name] = std::make_unique<Shader>(vPath, fPath);
    }

    Shader* Get(const std::string& name) {
        if (m_Shaders.find(name) != m_Shaders.end()) {
            return m_Shaders[name].get();
        }
        return nullptr;
    }
    
    void ReloadShaders()
    {
        for(auto it = m_Shaders.begin(); it != m_Shaders.end(); it++)
        {
            it->second->Reload();
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Shader>> m_Shaders;
};
