//
//  Scene.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 30/10/2025.
//
#include "Scene.h"
#include "Shader.h"
#include "Components.h"
#include "ECSSystems/RenderSystem.h"


Entity Scene::AddEntity(char* aName)
{
    
    Entity e = m_Coordinator.CreateEntity();

//     Add transform component
    TransformComponent transform{};
    transform.position = {0.0f, 0.0f, 0.0f};
    transform.rotation = {0.0f, 0.0f, 0.0f};
    transform.scale = {1.0f, 1.0f, 1.0f};
    m_Coordinator.AddComponent(e, transform);

    RenameEntity(e, aName);
    return e;
}

void Scene::RemoveEntity(Entity e)
{
    m_Coordinator.DestroyEntity(e);
}

void Scene::RenameEntity(Entity e, const char* newName)
{

    if(NameComponent* nameComponent = m_Coordinator.GetComponent<NameComponent>(e))
    {
        int count = NameExistCount(e, newName);
        if(count==0){
            nameComponent->Name = newName;
            return;
        }
        std::string name(newName);
        while(true){
            std::string suffix = "_"+std::to_string(count);
            std::string tempname = name + suffix;
            int n = NameExistCount(e, tempname.c_str());
            if(n == 0){
                nameComponent->Name = tempname;
                return;
            }
            else count++;
        }
    }

}

void Scene::Render(Shader& shader)
{
}

int Scene::NameExistCount(Entity e, const char* aName){
    int count = 0;
    const std::vector<Entity>& aliveEntities = m_Coordinator.GetAliveEntities();
    for(auto entity : aliveEntities){
        if(entity == e) continue;
        if(NameComponent* nameComponent = m_Coordinator.GetComponent<NameComponent>(entity)){
            if(nameComponent->Name == aName) count++;
        }
    }
    return count;
}
