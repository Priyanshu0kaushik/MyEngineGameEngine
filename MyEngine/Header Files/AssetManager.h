//
//  AssetManager.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 02/01/2026.
//

#pragma once
#include "TextureManager.h"
#include "MeshManager.h"
#include "MessageQueue.h"
#include <unordered_set>


class AssetManager{
public:
    static void Allocate();
    static AssetManager& Get();
    static void DeAllocate();

    TextureManager& GetTextureManager() { return m_TextureManager; }
    MeshManager& GetMeshManager() { return m_MeshManager; }
    
    AssetHandle GetAsset(const std::string& path);
    AssetHandle GetAsset(AssetType Type, const uint32_t iD);
    
    bool LoadAsset(const std::string& path, AssetHandle& result);
    void ProcessMessage(Message* msg);
    void SetMessageQueue(std::shared_ptr<MessageQueue> q) { messageQueue = q; }
    
    void AddAssetReference(const std::string& path, AssetType type);
    void RemoveAssetReference(const std::string& path, AssetType type);

    AssetType GetAssetTypeFromExtension(const std::string& path);

    void CleanUp();

private:
    static AssetManager* m_Instance;
    std::shared_ptr<MessageQueue> messageQueue;
    std::unordered_set<std::string> m_LoadingPaths;
    

    TextureManager m_TextureManager;
    MeshManager m_MeshManager;
};
