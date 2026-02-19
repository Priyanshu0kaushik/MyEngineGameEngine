//
//  DebugGizmosSystem.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 11/02/2026.
//

#pragma once
#include "ECS/ECSSystem.h"
#include "Components.h"
#include "Shader.h"
#include "ECS/Coordinator.h"

struct LineVertex {
    glm::vec3 position;
    glm::vec3 color;
};

class DebugGizmosSystem : public ECSSystem{
public:
    void Init() override;
    void Update(Entity selectedEntity);
    void Render();
    
private:
    void DrawSphereCollision(const Entity& entity);
    void DrawCircle(glm::vec3 center, float radius, glm::vec3 axis, glm::vec3 color);
    void DrawBoxCollision(const Entity& entity);
    void DrawBoxEdges(glm::vec3 v[8], glm::vec3 color);
    void Line(glm::vec3 start, glm::vec3 end, glm::vec3 color);
    
private:
    std::vector<LineVertex> lineVertices;
    unsigned int lineVBO, lineVAO;
    
    bool showAllCollision = false;
};
