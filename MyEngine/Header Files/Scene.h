//
//  Scene.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 30/10/2025.
//

#pragma once
#include <memory>
#include <vector>
#include "ECS/Coordinator.h"
#include "ECSSystems/RenderSystem.h"
#include "ECSSystems/CameraSystem.h"

class Shader;
class Scene{
public:
    Scene(Coordinator& coordinator, std::shared_ptr<RenderSystem> rs, std::shared_ptr<CameraSystem> cs)
            : m_Coordinator(coordinator), renderSystem(rs), cameraSystem(cs) {}
    
    Entity AddEntity(char* aName);
    void RemoveEntity(Entity e);
    
    void SyncLoadedAssets();
    bool SyncMaterial(Material& material);

    void RenameEntity(Entity e, const char* newName);
    
    void Save();
    void Load(const std::string& filePath);
private:
    int NameExistCount(Entity e, const char* aName);

    Coordinator& m_Coordinator;

    std::vector<Entity> mPendingMeshEntities{};

    std::shared_ptr<RenderSystem> renderSystem;
    std::shared_ptr<CameraSystem> cameraSystem;
};
