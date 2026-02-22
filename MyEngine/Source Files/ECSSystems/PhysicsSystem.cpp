//
//  PhysicsSystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 09/02/2026.
//

#include "ECSSystems/PhysicsSystem.h"
#include "ECS/Coordinator.h"
#include "ECSSystems/TerrainSystem.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void PhysicsSystem::Init()
{
    
}

// PHYSICS UPDATE LOOP
void PhysicsSystem::Update(float deltaTime)
{
    if(!m_Coordinator) return;
        
    for(Entity& entity : mEntities)
    {
        RigidBodyComponent* rb = m_Coordinator->GetComponent<RigidBodyComponent>(entity);
        TransformComponent* trans = m_Coordinator->GetComponent<TransformComponent>(entity);
        if(!rb || !trans || rb->isStatic) continue;

        // 1. gravity & acceleration
        if(!rb->isKinematic) {
            float gravity = -9.81f * rb->gravityScale;
            rb->velocity.y += gravity * deltaTime;
        }
        rb->velocity += rb->acceleration * deltaTime;

        // 2. applying damping
        float damping = std::pow(0.98f, deltaTime * 60.0f);
        rb->velocity *= damping;

        glm::vec3 nextPos = trans->position + (rb->velocity * deltaTime);

        // 4. terrain collision (for now we can only have one terrain)
        Entity terrainEntity = m_TerrainSystem->GetTerrainEntity();
        if (terrainEntity != UINT32_MAX) {
            float groundHeight = m_TerrainSystem->GetHeightAt(terrainEntity, nextPos.x, nextPos.z);
            
            float colliderOffset = 0.0f;
            if(auto* box = m_Coordinator->GetComponent<BoxColliderComponent>(entity)) {
                colliderOffset = box->extents.y;
            }

            if (nextPos.y - colliderOffset <= groundHeight + 0.001f)
            {
                nextPos.y = groundHeight + colliderOffset;
                if (rb->velocity.y < 0) {
                    rb->velocity.y = 0;
                }

                rb->velocity.x *= 0.95f;
                rb->velocity.z *= 0.95f;
            }
        }

        trans->SetPosition(nextPos);

        // acc reset
        rb->acceleration = glm::vec3(0.0f);
        if (glm::length(rb->velocity) < 0.01f) rb->velocity = glm::vec3(0.0f);

        // Update Bounding Boxes 
        if(m_Coordinator->GetComponent<ColliderComponent>(entity)) {
            UpdateBounds(entity);
        }
    }
    
    // COLLISION DETECTION
    // Right Now, brute force method, will switch to quad tree later
    for (auto itA = mEntities.begin(); itA != mEntities.end(); ++itA) {
        for (auto itB = std::next(itA); itB != mEntities.end(); ++itB) {
            
            auto* colliderA = m_Coordinator->GetComponent<ColliderComponent>(*itA);
            auto* colliderB = m_Coordinator->GetComponent<ColliderComponent>(*itB);
            
            auto* rbA = m_Coordinator->GetComponent<RigidBodyComponent>(*itA);
            auto* rbB = m_Coordinator->GetComponent<RigidBodyComponent>(*itB);
            
            if (rbA->isStatic && rbB->isStatic) continue;
            
            if (colliderA && colliderB)
            {
                ColliderType typeA = colliderA->type;
                ColliderType typeB = colliderB->type;
                
                // Dispatch to specific Narrowphase algorithms
                if (typeA == ColliderType::Box && typeB == ColliderType::Box)
                {
                    if(CheckBoxBoxCollision(*itA, *itB)) std::cout<<"Box Collided with Box\n";
                }
                else if (typeA == ColliderType::Sphere && typeB == ColliderType::Sphere)
                {
                    if(CheckSphereSphereCollision(*itA, *itB)) std::cout<<"Sphere Collided with Sphere\n";
                }
                else if (typeA == ColliderType::Sphere && typeB == ColliderType::Box)
                {
                    if(CheckSphereBoxCollision(*itA, *itB)) std::cout<<"Sphere Collided with Box\n";
                }
                else if (typeA == ColliderType::Box && typeB == ColliderType::Sphere)
                {
                    if(CheckSphereBoxCollision(*itB, *itA)) std::cout<<"Box Collided with Sphere\n";
                }
            }
        }
    }
}


// Updates the min/max bounds of a collider when the object moves or rotates.
void PhysicsSystem::UpdateBounds(Entity entity) {
    if(!m_Coordinator) return;
    auto* transform = m_Coordinator->GetComponent<TransformComponent>(entity);
    auto* collider = m_Coordinator->GetComponent<ColliderComponent>(entity);

    if (!transform || !collider) return;
         
    // Only recalculate if the object actually moved
    if(transform->isDirty || collider->isDirty){
        glm::mat4 entityMatrix = GetWorldMatrix(transform);
        collider->worldTransform = glm::translate(entityMatrix, collider->center);
        collider->worldInverse = glm::inverse(collider->worldTransform);
         
        if(collider->type == ColliderType::Box)
        {
            auto* boxCollider = m_Coordinator->GetComponent<BoxColliderComponent>(entity);
            if(!boxCollider) return;
            
            // Transform all 8 corners of the box to find the new World AABB
            glm::vec3 h = boxCollider->extents;
            glm::vec3 corners[8] = {
                {-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {-h.x, h.y, -h.z}, {h.x, h.y, -h.z},
                {-h.x, -h.y,  h.z}, {h.x, -h.y,  h.z}, {-h.x, h.y,  h.z}, {h.x, h.y,  h.z}
            };

            collider->worldMin = glm::vec3(FLT_MAX);
            collider->worldMax = glm::vec3(-FLT_MAX);

            for (int i = 0; i < 8; i++)
            {
                glm::vec3 worldPos = glm::vec3(collider->worldTransform * glm::vec4(corners[i], 1.0f));
                
                collider->worldMin = glm::min(collider->worldMin, worldPos);
                collider->worldMax = glm::max(collider->worldMax, worldPos);
            }
        }
        else if (collider->type == ColliderType::Sphere) {
            auto* sphere = m_Coordinator->GetComponent<SphereColliderComponent>(entity);
            if (!sphere) return;
            
            // Scale the radius uniformly by the largest scale component
            float maxScale = std::max({transform->scale.x, transform->scale.y, transform->scale.z});
            float r = sphere->radius * maxScale;
            glm::vec3 center = glm::vec3(collider->worldTransform[3]);
            
            collider->worldMin = center - glm::vec3(r);
            collider->worldMax = center + glm::vec3(r);
        }
    }
    transform->isDirty = false;
    collider->isDirty = false;
}

// SAT PROJECTION
// Helper function for Separating Axis Theorem.

void PhysicsSystem::ProjectBox(const ColliderComponent* col, const BoxColliderComponent* box, const glm::vec3& axis, float& min, float& max)
{

    glm::vec3 worldRight = glm::vec3(col->worldTransform[0]);
    glm::vec3 worldUp    = glm::vec3(col->worldTransform[1]);
    glm::vec3 worldForward = glm::vec3(col->worldTransform[2]);
    glm::vec3 worldCenter = glm::vec3(col->worldTransform[3]);

    float centerProj = glm::dot(worldCenter, axis);

    // Calculate projected radius
    float radius =  box->extents.x * std::abs(glm::dot(worldRight, axis)) +
                    box->extents.y * std::abs(glm::dot(worldUp, axis)) +
                    box->extents.z * std::abs(glm::dot(worldForward, axis));

    min = centerProj - radius;
    max = centerProj + radius;
}

// BOX vs BOX COLLISION (Separating Axis Theorem)
// First we see using AABB, then if needed we check using SAT
bool PhysicsSystem::CheckBoxBoxCollision(Entity entityA, Entity entityB) {
    if(!m_Coordinator) return;
    
    auto* colA = m_Coordinator->GetComponent<ColliderComponent>(entityA);
    auto* colB = m_Coordinator->GetComponent<ColliderComponent>(entityB);
    colA->isColliding = false;
    colB->isColliding = false;
    
    if(entityA == entityB) return false;
    auto* boxA = m_Coordinator->GetComponent<BoxColliderComponent>(entityA);
    auto* boxB = m_Coordinator->GetComponent<BoxColliderComponent>(entityB);
    
    if(!boxA || !boxB) return false;
         
    // 1. Broadphase check (AABB)
    // Fast fail: If the world-aligned boxes don't touch, the rotated ones definitely don't.
    if (colA->worldMax.x < colB->worldMin.x || colA->worldMin.x > colB->worldMax.x ||
        colA->worldMax.y < colB->worldMin.y || colA->worldMin.y > colB->worldMax.y ||
        colA->worldMax.z < colB->worldMin.z || colA->worldMin.z > colB->worldMax.z)
    {
        return false;
    }
    
    int counter = 0;
    glm::vec3 axes[15];
    
    // 3 Face Normals from A
    axes[counter++] = glm::normalize(glm::vec3(colA->worldTransform[0]));
    axes[counter++] = glm::normalize(glm::vec3(colA->worldTransform[1]));
    axes[counter++] = glm::normalize(glm::vec3(colA->worldTransform[2]));

    // 3 Face Normals from B
    axes[counter++] = glm::normalize(glm::vec3(colB->worldTransform[0]));
    axes[counter++] = glm::normalize(glm::vec3(colB->worldTransform[1]));
    axes[counter++] = glm::normalize(glm::vec3(colB->worldTransform[2]));
    
    // 9 Edge Cross Products (Edges of A crossed with Edges of B)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            glm::vec3 cross = glm::cross(axes[i], axes[3 + j]);
            if (glm::length(cross) > 0.001f) {
                axes[counter++] = glm::normalize(cross);
            }
        }
    }
    
    float minOverlap = FLT_MAX;
    glm::vec3 collisionNormal;

    // Test all axes
    for(int i = 0; i < counter; ++i) {
        float minA, maxA, minB, maxB;
        ProjectBox(colA, boxA, axes[i], minA, maxA);
        ProjectBox(colB, boxB, axes[i], minB, maxB);

        // Gap found! No collision.
        if (maxA < minB || maxB < minA) return false;

        // Calculate overlap
        float overlap = std::min(maxA, maxB) - std::max(minA, minB);
        
        // MTV (Minimum Translation Vector) Logic
        // We want to push the object out along the minimum overlap axis
        if (overlap < minOverlap) {
            minOverlap = overlap;
            collisionNormal = axes[i];
        }
    }

    // COLLISION RESOLUTION
    // All Axis Overlapped. Now we push objects apart.
    if (minOverlap > 0.0001f) {
         
        // Ensure normal points from A to B
        glm::vec3 centerA = glm::vec3(colA->worldTransform[3]);
        glm::vec3 centerB = glm::vec3(colB->worldTransform[3]);
        glm::vec3 dir = centerB - centerA;
         
        if (glm::length(collisionNormal) < 0.0001f) collisionNormal = glm::vec3(0, 1, 0);
        else collisionNormal = glm::normalize(collisionNormal);
         
        if (glm::dot(dir, collisionNormal) < 0)
            collisionNormal = -collisionNormal;
         
        auto* transformA = m_Coordinator->GetComponent<TransformComponent>(entityA);
        auto* transformB = m_Coordinator->GetComponent<TransformComponent>(entityB);
        auto* rbA = m_Coordinator->GetComponent<RigidBodyComponent>(entityA);
        auto* rbB = m_Coordinator->GetComponent<RigidBodyComponent>(entityB);
         
        float invMassA = (rbA && !rbA->isStatic) ? 1.0f / rbA->mass : 0.0f;
        float invMassB = (rbB && !rbB->isStatic) ? 1.0f / rbB->mass : 0.0f;
         
        float totalInvMass = invMassA + invMassB;
         
        if (totalInvMass > 0.0001f) {
            float percent = 0.8f;
            float slop = 0.01f;
             
            glm::vec3 push = collisionNormal * (std::max(minOverlap - slop, 0.0f) / totalInvMass) * percent;
             
            if (invMassA > 0) transformA->SetPosition(transformA->position - push * invMassA);
            if (invMassB > 0) transformB->SetPosition(transformB->position + push * invMassB);
        }
         
        // Reflection (Bouncing)
        if (rbA && !rbA->isStatic && !rbA->isKinematic) {
            float velAlongNormal = glm::dot(rbA->velocity, collisionNormal);
            // Only reflect if moving towards the collider
            if (velAlongNormal > 0) {
                rbA->velocity = glm::reflect(rbA->velocity, collisionNormal) * colA->bounciness;
            }
        }
        if (rbA && !rbA->isStatic && !rbA->isKinematic)
            rbA->velocity = glm::reflect(rbA->velocity, collisionNormal) * colA->bounciness;
         
        colA->isColliding = true;
        colB->isColliding = true;
        return true;
    }
    return false;
    
}

// SPHERE vs SPHERE
// Simple distance check
bool PhysicsSystem::CheckSphereSphereCollision(Entity entityA, Entity entityB) {
    if(!m_Coordinator) return;
    
    auto* colA = m_Coordinator->GetComponent<ColliderComponent>(entityA);
    auto* colB = m_Coordinator->GetComponent<ColliderComponent>(entityB);
    auto* sphereA = m_Coordinator->GetComponent<SphereColliderComponent>(entityA);
    auto* sphereB = m_Coordinator->GetComponent<SphereColliderComponent>(entityB);

    if (!sphereA || !sphereB) return false;

    glm::vec3 posA = glm::vec3(colA->worldTransform[3]);
    glm::vec3 posB = glm::vec3(colB->worldTransform[3]);

    glm::vec3 delta = posA - posB;
    // Compare distance squared with radius squared
    float distSq = glm::dot(delta, delta);
    float sumRadii = sphereA->radius + sphereB->radius;

    if (distSq < (sumRadii * sumRadii)) {
        colA->isColliding = true;
        colB->isColliding = true;
        return true;
    }

    return false;
}

// SPHERE vs BOX
// Find Closest Point on AABB -> Distance Check.
bool PhysicsSystem::CheckSphereBoxCollision(Entity sphereEnt, Entity boxEnt) {
    if(!m_Coordinator) return;
    
    auto* sphereCol = m_Coordinator->GetComponent<ColliderComponent>(sphereEnt);
    auto* sphereData = m_Coordinator->GetComponent<SphereColliderComponent>(sphereEnt);
    auto* boxCol = m_Coordinator->GetComponent<ColliderComponent>(boxEnt);
    auto* boxData = m_Coordinator->GetComponent<BoxColliderComponent>(boxEnt);
    
    // Transform Sphere Center into Box's Local Coordinate Space
    glm::mat4 boxInv = boxCol->worldInverse;
    glm::vec3 sphereCenterWorld = glm::vec3(sphereCol->worldTransform[3]);
    glm::vec3 localSphereCenter = glm::vec3(boxInv * glm::vec4(sphereCenterWorld, 1.0f));
    
    // Find closest point on the Box to the Sphere center (Clamp)
    glm::vec3 closestPoint;
    closestPoint.x = glm::clamp(localSphereCenter.x, -boxData->extents.x, boxData->extents.x);
    closestPoint.y = glm::clamp(localSphereCenter.y, -boxData->extents.y, boxData->extents.y);
    closestPoint.z = glm::clamp(localSphereCenter.z, -boxData->extents.z, boxData->extents.z);
    
    // Check distance
    float distance = glm::distance(localSphereCenter, closestPoint);
    
    if (distance < sphereData->radius) {
        float overlap = sphereData->radius - distance;
         
        // Calculate collision normal in Local Space
        glm::vec3 localNormal = glm::normalize(localSphereCenter - closestPoint);
         
        // Transform normal back to World Space
        glm::vec3 worldNormal = glm::normalize(glm::vec3(boxCol->worldTransform * glm::vec4(localNormal, 0.0f)));
         
        
        auto* sphereTransform = m_Coordinator->GetComponent<TransformComponent>(sphereEnt);
        sphereTransform->SetPosition(sphereTransform->position + worldNormal * overlap);
         
        // Bounce
        auto* rb = m_Coordinator->GetComponent<RigidBodyComponent>(sphereEnt);
        if(rb && !rb->isKinematic) {
            rb->velocity = glm::reflect(rb->velocity, worldNormal) * sphereCol->bounciness;
        }
         
        sphereCol->isColliding = true;
        boxCol->isColliding = true;
        return true;
    }
    return false;
}

glm::mat4 PhysicsSystem::GetWorldMatrix(const TransformComponent* transform) {
    if(!transform) return;
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, transform->position);
    
    model = glm::rotate(model, glm::radians(transform->rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(transform->rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(transform->rotation.z), glm::vec3(0, 0, 1));

    model = glm::scale(model, transform->scale);
    return model;
}
