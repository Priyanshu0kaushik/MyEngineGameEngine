//
//  ECSSystemManager.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 17/11/2025.
//
#pragma once
#include "ECS.h"
#include "ECSSystem.h"
#include <map>

class ECSSystemManager
{
public:
    template<typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        const char* typeName = typeid(T).name();

        auto system = std::make_shared<T>();
        mSystems.insert({typeName, std::static_pointer_cast<ECSSystem>(system)});
        return system;
    }

    template<typename T>
    void SetSignature(Signature signature)
    {
        const char* typeName = typeid(T).name();

        mSignatures.insert({typeName, signature});
    }

    void EntityDestroyed(Entity entity)
    {
        for (auto const& pair : mSystems)
        {
            auto const& system = pair.second;
            auto& entities = system->mEntities;
            entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
        }
    }

    void EntitySignatureChanged(Entity entity, Signature entitySignature)
    {
        for (auto const& pair : mSystems)
        {
            auto const& type = pair.first;
            auto const& system = pair.second;
            auto const& systemSignature = mSignatures[type];

            if ((entitySignature & systemSignature) == systemSignature)
            {
                auto& entities = system->mEntities;
                            
                if (std::find(entities.begin(), entities.end(), entity) == entities.end())
                {
                    system->mEntities.push_back(entity);
                }
            }
            else
            {
                auto& entities = system->mEntities;
                entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
            }
        }
    }

private:
    std::unordered_map<const char*, Signature> mSignatures{};

    std::unordered_map<const char*, std::shared_ptr<ECSSystem>> mSystems{};
};
