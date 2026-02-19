//
//  RenderSystem.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 19/11/2025.
//

#pragma once
#include "ECS/ECSSystem.h"
#include "Shader.h"
#include "Components.h"
#include "ECS/Coordinator.h"

class LightSystem : public ECSSystem{
public:
    void Init() override;
    void Render(Shader& shader);
    glm::mat4 GetLightSpaceMatrix();
private:
    glm::vec3 m_LastDirectionalDir = glm::vec3(0.0f, -1.0f, 0.0f);
};
