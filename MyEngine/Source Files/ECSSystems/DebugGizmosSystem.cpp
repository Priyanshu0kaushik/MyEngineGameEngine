//
//  DebugGizmosSystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 11/02/2026.
//

#include "ECSSystems/DebugGizmosSystem.h"
#include "ECS/Coordinator.h"
#include "GLAD/include/glad/glad.h"

void DebugGizmosSystem::Init()
{
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
    // Color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void DebugGizmosSystem::Update(Entity selectedEntity)
{
    if(!m_Coordinator) return;
    if(!showAllCollision)
    {
        ColliderComponent* collider = m_Coordinator->GetComponent<ColliderComponent>(selectedEntity);
        if(!collider) return;
        ColliderType collisionType = collider->type;
        if(collisionType == ColliderType::Box){
            DrawBoxCollision(selectedEntity);
        }
        else if(collisionType == ColliderType::Sphere){
            DrawSphereCollision(selectedEntity);
        }
        
    }
    else{
        for (auto const& entity : mEntities)
        {
            ColliderComponent* collider = m_Coordinator->GetComponent<ColliderComponent>(selectedEntity);
            if(!collider) return;
            ColliderType collisionType = collider->type;
            if(collisionType == ColliderType::Box){
                DrawBoxCollision(entity);
            }
            else if(collisionType == ColliderType::Sphere){
                DrawSphereCollision(entity);
            }
        }
    }
    
}

void DebugGizmosSystem::DrawSphereCollision(const Entity& entity)
{
    auto* transform = m_Coordinator->GetComponent<TransformComponent>(entity);
    auto* collider = m_Coordinator->GetComponent<ColliderComponent>(entity);
    auto* sphere = m_Coordinator->GetComponent<SphereColliderComponent>(entity);

    
    if(!sphere || !collider || !transform) return;
    glm::vec3 color = collider->isColliding ? glm::vec3(1,0,0) : glm::vec3(0, 1, 0);
    
    glm::vec3 worldCenter = glm::vec3(collider->worldTransform[3]);
    
    float maxScale = std::max({transform->scale.x, transform->scale.y, transform->scale.z});
    float worldRadius = sphere->radius * maxScale;

    DrawCircle(worldCenter, worldRadius, glm::vec3(1, 0, 0), color); // X-axis normal
    DrawCircle(worldCenter, worldRadius, glm::vec3(0, 1, 0), color); // Y-axis normal
    DrawCircle(worldCenter, worldRadius, glm::vec3(0, 0, 1), color); // Z-axis normal
    
}

void DebugGizmosSystem::DrawCircle(glm::vec3 center, float radius, glm::vec3 axis, glm::vec3 color) {
    const int segments = 32;
    std::vector<glm::vec3> points;

    glm::vec3 v1, v2;
    if (std::abs(axis.x) > std::abs(axis.z))
        v1 = glm::vec3(-axis.y, axis.x, 0.0f);
    else
        v1 = glm::vec3(0.0f, -axis.z, axis.y);
    
    v1 = glm::normalize(v1);
    v2 = glm::cross(axis, v1);

    for (int i = 0; i <= segments; i++)
    {
        float angle = (float)i / segments * 2.0f * 3.14f;
        glm::vec3 p = center + (v1 * std::cos(angle) + v2 * std::sin(angle)) * radius;
        points.push_back(p);
    }

    for (int i = 0; i < segments; i++) {
        lineVertices.push_back({points[i], color});
        lineVertices.push_back({points[i+1], color});
    }
}

void DebugGizmosSystem::DrawBoxCollision(const Entity& entity)
{
    auto* col = m_Coordinator->GetComponent<ColliderComponent>(entity);
    auto* box = m_Coordinator->GetComponent<BoxColliderComponent>(entity);
    
    if(!box || !col) return;
    glm::vec3 color = col->isColliding ? glm::vec3(1,0,0) : glm::vec3(0, 1, 0);

    glm::vec3 h = box->extents;
    glm::vec3 v[8] = {
        {-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {h.x, h.y, -h.z}, {-h.x, h.y, -h.z},
        {-h.x, -h.y,  h.z}, {h.x, -h.y,  h.z}, {h.x, h.y,  h.z}, {-h.x, h.y,  h.z}
    };

    for(int i = 0; i < 8; i++) {
        v[i] = glm::vec3(col->worldTransform * glm::vec4(v[i], 1.0f));
    }

    DrawBoxEdges(v, color);
}


void DebugGizmosSystem::Render() {
    if (lineVertices.empty()) return;

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(LineVertex), lineVertices.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(lineVAO);
    glDrawArrays(GL_LINES, 0, (GLsizei)lineVertices.size());

    lineVertices.clear();
}

void DebugGizmosSystem::Line(glm::vec3 start, glm::vec3 end, glm::vec3 color) {
    lineVertices.push_back({start, color});
    lineVertices.push_back({end, color});
}

void DebugGizmosSystem::DrawBoxEdges(glm::vec3 v[8], glm::vec3 color) {
    // Bottom Face
    Line(v[0], v[1], color); Line(v[1], v[2], color);
    Line(v[2], v[3], color); Line(v[3], v[0], color);

    // Top Face
    Line(v[4], v[5], color); Line(v[5], v[6], color);
    Line(v[6], v[7], color); Line(v[7], v[4], color);

    // Vertical Edges
    Line(v[0], v[4], color); Line(v[1], v[5], color);
    Line(v[2], v[6], color); Line(v[3], v[7], color);
}
