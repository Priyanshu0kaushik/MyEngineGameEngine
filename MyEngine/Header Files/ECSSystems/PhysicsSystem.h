//
//  PhysicsSystem.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 09/02/2026.
//

#pragma once
#include "ECS/ECSSystem.h"
#include "ECSSystems/TerrainSystem.h"
#include "Components.h"
#include "ECS/Coordinator.h"

class PhysicsSystem : public ECSSystem{
public:
    void Init() override;
    void Update(float deltaTime);
    void UpdateBounds(Entity entity);
    
    void SetTerrainSystem(std::shared_ptr<TerrainSystem> terrainSystem){
        m_TerrainSystem = terrainSystem;
    }
private:
    glm::mat4 GetWorldMatrix(const TransformComponent* transform);
    bool CheckSphereSphereCollision(Entity entityA, Entity entityB);
    bool CheckBoxBoxCollision(Entity entityA, Entity entityB);
    bool CheckSphereBoxCollision(Entity sphereEnt, Entity boxEnt);
    void ProjectBox(const ColliderComponent* col, const BoxColliderComponent* box, const glm::vec3& axis, float& min, float& max);
private:
    
    std::shared_ptr<TerrainSystem> m_TerrainSystem;

    glm::vec3 axes[15];
};
