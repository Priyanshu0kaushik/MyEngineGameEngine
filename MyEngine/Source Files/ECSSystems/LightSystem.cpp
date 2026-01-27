//
//  LightSystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 14/01/2026.
//

#include "ECSSystems/LightSystem.h"
#include "ECS/Coordinator.h"
#include "AssetManager.h"
#include "GLAD/include/glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void LightSystem::Init()
{
}

void LightSystem::Render(Shader &shader){

    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> diffuses;
    std::vector<glm::vec4> speculars;
    std::vector<glm::vec3> attenuations;

    int count = 0;
    shader.SetVec4("u_Light_ambient", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
    
    for (auto const& entity : mEntities) {
        if (count >= 100) break;

        auto* transform = m_Coordinator->GetComponent<TransformComponent>(entity);
        auto* light = m_Coordinator->GetComponent<LightComponent>(entity);
        glm::vec4 posType;

        if (light->type == LightType::Directional)
        {
            glm::mat4 rot = glm::mat4(1.0f);
            rot = glm::rotate(rot, glm::radians(transform->rotation.y), glm::vec3(0, 1, 0));
            rot = glm::rotate(rot, glm::radians(transform->rotation.x), glm::vec3(1, 0, 0));
            rot = glm::rotate(rot, glm::radians(transform->rotation.z), glm::vec3(0, 0, 1));

            glm::vec3 direction = glm::vec3(rot * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
            m_LastDirectionalDir = direction;
            posType = glm::vec4(direction, 0.0f);
        }
        else {
            posType = glm::vec4(transform->position, (float)light->type);
        }

        positions.push_back(posType);
        diffuses.push_back(glm::vec4(light->color * light->intensity, 1.0f));
        speculars.push_back(glm::vec4(light->color * light->intensity, 1.0f));
        attenuations.push_back(glm::vec3(light->constant, light->linear, light->quadratic));
        
        count++;
    }

    shader.SetInt("u_LightCount", count);
    shader.SetVec4("u_Light_ambient", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
    if (count == 0) {
        shader.SetInt("u_LightCount", 1);
        shader.SetVec4("u_Light_position[0]", glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
        shader.SetVec4("u_Light_diffuse[0]", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    else
    {
        shader.SetVec4Array("u_Light_position", positions);
        shader.SetVec4Array("u_Light_diffuse", diffuses);
        shader.SetVec4Array("u_Light_specular", speculars);
        shader.SetVec3Array("u_Light_attenuation", attenuations);
    }
    
    shader.SetVec4("u_Light_ambient", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));

}

glm::mat4 LightSystem::GetLightSpaceMatrix()
{
    glm::mat4 biasMatrix(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
    );
    
    float orthoSize = 20.0f;
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 50.0f);
    
    glm::vec3 lightPos = -m_LastDirectionalDir * 20.0f;
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    return biasMatrix * lightProjection * lightView;
}
