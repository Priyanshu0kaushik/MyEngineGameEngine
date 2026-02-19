//
//  ScriptSystem.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 17/02/2026.
//

#pragma once
#include "ECS/ECSSystem.h"
#include "Components.h"
#include "ScriptManager.h"

class ScriptSystem : public ECSSystem{
public:
    void Init() override;
    void Update(float deltaTime);
private:
    std::unique_ptr<ScriptManager> m_ScriptManager;
};
