//
//  Coordinator.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 17/11/2025.
//

#pragma once
#include <memory>
#include "ComponentManager.h"
#include "EntityManager.h"
#include "ECSSystemManager.h"
#include "Components.h"

class Coordinator
{
public:
    void Init()
    {
        mComponentManager = std::make_unique<ComponentManager>();
        mEntityManager = std::make_unique<EntityManager>();
        mSystemManager = std::make_unique<ECSSystemManager>();
        RegisterComponent<NameComponent>();
    }


    Entity CreateEntity()
    {
        Entity e = mEntityManager->CreateEntity();
        NameComponent nameComponent;
        nameComponent.Name = "Entity";
        AddComponent(e, nameComponent);
        return e;
    }

    void DestroyEntity(Entity entity)
    {
        mEntityManager->DestroyEntity(entity);

        mComponentManager->EntityDestroyed(entity);

        mSystemManager->EntityDestroyed(entity);
    }


    template<typename T>
    void RegisterComponent()
    {
        mComponentManager->RegisterComponent<T>();
        mComponentCreators[T::TypeName] = [this](Entity e)
        {
            T component{};
            this->AddComponent<T>(e, component);
        };
    }

    template<typename T>
    bool AddComponent(Entity entity, T component)
    {
        if(T::UniquePerEntity){
            if (GetComponent<T>(entity)!=nullptr) return false;
        }
        
        mComponentManager->AddComponent<T>(entity, component);

        auto signature = mEntityManager->GetSignature(entity);
        signature.set(mComponentManager->GetComponentType<T>(), true);
        mEntityManager->SetSignature(entity, signature);

        mSystemManager->EntitySignatureChanged(entity, signature);
        return true;
    }
    
    void AddComponentByName(Entity e,const char* ComponentName)
    {
        auto it = mComponentCreators.find(ComponentName);
        if (it != mComponentCreators.end())
        {
            it->second(e);
        }
    }

    template<typename T>
    void RemoveComponent(Entity entity)
    {
        mComponentManager->RemoveComponent<T>(entity);

        auto signature = mEntityManager->GetSignature(entity);
        signature.set(mComponentManager->GetComponentType<T>(), false);
        mEntityManager->SetSignature(entity, signature);

        mSystemManager->EntitySignatureChanged(entity, signature);
    }

    template<typename T>
    T* GetComponent(Entity entity)
    {
        return mComponentManager->GetComponent<T>(entity);
    }

    template<typename T>
    ComponentType GetComponentType()
    {
        return mComponentManager->GetComponentType<T>();
    }

    template<typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray()
    {
        mComponentManager->GetComponentArray<T>();
    }

    template<typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        return mSystemManager->RegisterSystem<T>();
    }

    template<typename T>
    void SetSystemSignature(Signature signature)
    {
        mSystemManager->SetSignature<T>(signature);
    }
    
    const std::vector<Entity>& GetAliveEntities() const
    {
        return mEntityManager->GetAliveEntities();
    }
    
    bool DoesEntityExist(Entity e){
        return mEntityManager->DoesEntityExist(e);
    }
    
    std::vector<std::string> GetComponentNames()
    {
        return mComponentManager->GetComponentNames();
    }
    
    Entity CopyComponents(Entity sourceEntity, Entity newEntity)
    {

        auto* sourceTrans = GetComponent<TransformComponent>(sourceEntity);
        auto* newTrans = GetComponent<TransformComponent>(newEntity);
        if (sourceTrans) {
            *newTrans = *sourceTrans;
        }

        auto* sourceMesh = GetComponent<MeshComponent>(sourceEntity);
        if (sourceMesh) {
            AddComponent(newEntity, *sourceMesh);
        }

        auto* sourceRB = GetComponent<RigidBodyComponent>(sourceEntity);
        if (sourceRB) {
            AddComponent(newEntity, *sourceRB);
        }

        auto* sourceBox = GetComponent<BoxColliderComponent>(sourceEntity);
        if (sourceBox) {
            AddComponent(newEntity, *sourceBox);
        }
        
        auto* sourceSphere = GetComponent<SphereColliderComponent>(sourceEntity);
        if (sourceSphere) {
            AddComponent(newEntity, *sourceSphere);
        }
        
        auto* sourceCol = GetComponent<ColliderComponent>(sourceEntity);
        if (sourceCol) {
            AddComponent(newEntity, *sourceCol);
        }
        
        auto* sourceScript = GetComponent<ScriptComponent>(sourceEntity);
        if(sourceScript){
            AddComponent(newEntity, *sourceScript);
        }
        return newEntity;
    }
    
private:
    std::unordered_map<std::string, std::function<void(Entity)>> mComponentCreators;
    std::unique_ptr<ComponentManager> mComponentManager;
    std::unique_ptr<EntityManager> mEntityManager;
    std::unique_ptr<ECSSystemManager> mSystemManager;
};
