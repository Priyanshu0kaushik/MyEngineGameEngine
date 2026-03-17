//
//  ComponentArray.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 16/11/2025.
//

#pragma once
#include "ECS.h"
#include <iostream>
#include <array>       
#include <unordered_map>
#include <cstddef>

class IComponentArray
{
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};


template<typename T>
class ComponentArray : public IComponentArray
{
public:
    void InsertData(Entity entity, T component)
    {

        size_t newIndex = mSize;
        mEntityToIndexMap[entity] = newIndex;
        mIndexToEntityMap[newIndex] = entity;
        mComponentArray[newIndex] = component;
        ++mSize;
    }

    void RemoveData(Entity entity)
    {

        size_t indexOfRemovedEntity = mEntityToIndexMap[entity];
        size_t indexOfLastElement = mSize - 1;
        mComponentArray[indexOfRemovedEntity] = mComponentArray[indexOfLastElement];

        Entity entityOfLastElement = mIndexToEntityMap[indexOfLastElement];
        mEntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
        mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

        mEntityToIndexMap.erase(entity);
        mIndexToEntityMap.erase(indexOfLastElement);

        --mSize;
    }

    T* GetData(Entity entity)
    {
        auto it = mEntityToIndexMap.find(entity);
        if (it == mEntityToIndexMap.end())
            return nullptr;
        return &mComponentArray[it->second];
    }

    void EntityDestroyed(Entity entity) override
    {
        if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end())
        {
            RemoveData(entity);
        }
    }
private:
    std::array<T, MAX_ENTITIES> mComponentArray;

    std::unordered_map<Entity, size_t> mEntityToIndexMap;

    std::unordered_map<size_t, Entity> mIndexToEntityMap;

    size_t mSize;
};
