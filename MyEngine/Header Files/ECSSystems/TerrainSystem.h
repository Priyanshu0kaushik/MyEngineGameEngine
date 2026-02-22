//
//  TerrainSystem.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 21/02/2026.
//

#pragma once
#include "ECS/ECSSystem.h"
#include "Components.h"

class Shader;

class TerrainSystem : public ECSSystem{
public:
    void Init() override;
    
    void Render(Shader& shader);
    void CreateTerrain(Entity entity);
    float GetHeightAt(Entity entity, float worldX, float worldZ);

    Entity GetTerrainEntity(){return m_TerrainEntity;}
private:
    glm::mat4 GetWorldMatrix(const TransformComponent* transform);
    void GenerateMesh(Entity entity);
    
private:
    Entity m_TerrainEntity = UINT32_MAX;
};
