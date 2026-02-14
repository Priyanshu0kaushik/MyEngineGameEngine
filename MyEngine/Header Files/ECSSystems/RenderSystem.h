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

class RenderSystem : public ECSSystem{
public:
    void Init() override;
    void Render(Shader& shader);
    void SetCoordinator(Coordinator* aCoordinator){m_Coordinator = aCoordinator;}
private:
    void DrawPhysicsGizmos();
    void UploadMeshIfNeeded(Entity e, MeshComponent* mc);
    glm::mat4 BuildModelMatrix(TransformComponent* t);
    
private:
    Coordinator* m_Coordinator;
};
