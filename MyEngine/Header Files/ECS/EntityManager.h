//
//  EntityManager.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 16/11/2025.
//

#pragma once
#include "ECS.h"
#include <queue>
#include <array>

class EntityManager
{
public:
    EntityManager()
    {
        
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
        {
            mAvailableEntities.push(entity);
        }
    }

    Entity CreateEntity()
    {

        Entity id = mAvailableEntities.front();
        mAvailableEntities.pop();
        mAliveEntities.push_back(id);
        ++mLivingEntityCount;

        return id;
    }

    void DestroyEntity(Entity entity)
    {
        mSignatures[entity].reset();
        mAliveEntities.erase(std::remove(mAliveEntities.begin(), mAliveEntities.end(), entity), mAliveEntities.end());
        mAvailableEntities.push(entity);
        --mLivingEntityCount;
    }

    void SetSignature(Entity entity, Signature signature)
    {

        mSignatures[entity] = signature;
    }

    Signature GetSignature(Entity entity)
    {
        return mSignatures[entity];
    }
    
    const std::vector<Entity>& GetAliveEntities() const{return mAliveEntities;}
    
    bool DoesEntityExist(Entity e){
        return std::find(mAliveEntities.begin(), mAliveEntities.end(), e) != mAliveEntities.end();
    }

private:
    std::queue<Entity> mAvailableEntities{};

    std::array<Signature, MAX_ENTITIES> mSignatures{};

    uint32_t mLivingEntityCount{};
    
    std::vector<Entity> mAliveEntities;

};
