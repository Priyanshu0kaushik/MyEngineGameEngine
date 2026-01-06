//
//  AssetManager.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 03/01/2026.
//

#include "AssetManager.h"
#include <filesystem>
#include <algorithm>
#include <iostream>

AssetManager* AssetManager::m_Instance = nullptr;


void AssetManager::Allocate()
{
    assert(m_Instance == nullptr);
    if (m_Instance) return;
    m_Instance = new AssetManager();
}

void AssetManager::DeAllocate(){
    delete m_Instance;
}

AssetManager& AssetManager::Get()
{
    return *m_Instance;
};


std::string ToLowerCase(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

AssetType AssetManager::GetAssetTypeFromExtension(const std::string &path)
{
    std::filesystem::path filePath(path);
    if (!filePath.has_extension()) return AssetType::None;
    
    std::string ext = ToLowerCase(filePath.extension().string());
    
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp")
    {
        return AssetType::Texture;
    }
        
    if (ext == ".obj")
    {
        return AssetType::Mesh;
    }
    
    return AssetType::None;
}

bool AssetManager::LoadAsset(const std::string& path, AssetHandle& result)
{
    AssetType type = GetAssetTypeFromExtension(path);
    switch (type) {
        case AssetType::Texture:
            std::cout << "[AssetManager] Detected Texture: " << path << std::endl;
            if(m_TextureManager.LoadTexture(path.c_str(), static_cast<TextureData*>(result.Data))){
                uint32_t iD = m_TextureManager.CreateTexture(static_cast<TextureData*>(result.Data));
                m_TextureManager.RegisterTexture(path.c_str(), iD);
                result.IsReady = true;
                return true;
            }
            else return false;
            

        case AssetType::Mesh:
            std::cout << "[AssetManager] Detected Mesh: " << path << std::endl;
            if(m_MeshManager.LoadMesh(path.c_str(), static_cast<Mesh*>(result.Data))){
                uint32_t iD = m_MeshManager.CreateMesh(static_cast<Mesh*>(result.Data));
                m_MeshManager.RegisterMesh(path.c_str(), iD);
                result.IsReady = true;
                return true;
            }
            else return false;

            

        case AssetType::Material:
            std::cout << "[AssetManager] Detected Material: " << path << std::endl;
            
            break;

        default:
            std::cerr << "[AssetManager] Unknown file type: " << path << std::endl;
            break;
    }
    return false;
}

AssetHandle AssetManager::GetAsset(const std::string &path){
    AssetHandle result;
    AssetType type = GetAssetTypeFromExtension(path);
    switch(type){
        case AssetType::Texture:
            std::cout << "[AssetManager] Detected Texture: " << path << std::endl;
            result = m_TextureManager.GetTexture(path);
            if(!result.Data){
                result.Data = new TextureData();
                messageQueue->Push(std::make_unique<LoadAssetMessage>(path, result));
            }
            break;
        case AssetType::Mesh:
            std::cout << "[AssetManager] Detected Mesh: " << path << std::endl;
            result = m_MeshManager.GetMesh(path);
            if(!result.Data){
                result.Data = new Mesh();
                messageQueue->Push(std::make_unique<LoadAssetMessage>(path, result));
            }
            break;
        case AssetType::Material:
            std::cout << "[AssetManager] Detected Material: " << path << std::endl;
            break;

        default:
            std::cerr << "[AssetManager] Unknown file type: " << path << std::endl;
            break;
    }
    return result;
}

void AssetManager::ProcessMessage(Message *msg)
{
    if (msg->type == MessageType::LoadAsset)
    {
        auto loadMsg = static_cast<LoadAssetMessage*>(msg);
        
        bool success = LoadAsset(loadMsg->path, loadMsg->assetHandle);
        
        AssetType type = GetAssetTypeFromExtension(loadMsg->path);
        
        if (success)
        {
            loadMsg->assetHandle.IsReady = true;
            std::unique_ptr<AssetLoadedMessage> replyMsg = std::make_unique<AssetLoadedMessage>(loadMsg->path, type);
            messageQueue->Push(std::move(replyMsg));
        }
    }
}
