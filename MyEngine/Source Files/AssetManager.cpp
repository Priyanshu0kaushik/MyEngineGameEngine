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

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <pthread.h>
#else
#include <pthread.h>
#include <sys/prctl.h>
#endif

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

void AssetManager::Init()
{
    m_WorkerThread = std::thread(&AssetManager::WorkerLoop, this);
}

void AssetManager::WorkerLoop() {
    const char* threadName = "AssetLoaderWorker";

    #if defined(__APPLE__)
        pthread_setname_np(threadName);
    #elif defined(__linux__)
        pthread_setname_np(pthread_self(), threadName);
    #elif defined(_WIN32)
        typedef HRESULT (WINAPI *TSetThreadDescription)(HANDLE, PCWSTR);
        TSetThreadDescription pSetThreadDescription = (TSetThreadDescription)GetProcAddress(GetModuleHandleA("kernel32.dll"), "SetThreadDescription");
        if (pSetThreadDescription) {
            wchar_t wName[64];
            mbstowcs(wName, threadName, 64);
            pSetThreadDescription(GetCurrentThread(), wName);
        }
    #endif
    
    while (m_Running) {
        auto msg = messageQueue->WaitAndPop();
        if (msg->type == MessageType::LoadAsset) {
            auto loadMsg = static_cast<LoadAssetMessage*>(msg.get());
            AssetType type = GetAssetTypeFromExtension(loadMsg->path);

            if (type == AssetType::Mesh) {
                bool success = LoadAsset(loadMsg->path, loadMsg->assetHandle);
                if (success) {
                    auto reply = std::make_unique<AssetLoadedMessage>(loadMsg->path, type);
                }
            } else {
                messageQueue->Push(std::move(msg));
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
}


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
        std::cout << "[AssetManager-LoadAsset] Detected Texture: " << path << std::endl;
        if(m_TextureManager.LoadTexture(path.c_str(), static_cast<TextureData*>(result.Data))){
            uint32_t iD = m_TextureManager.CreateTexture(static_cast<TextureData*>(result.Data));
            m_TextureManager.RegisterTexture(path.c_str(), iD);
            result.iD = iD;
            result.IsReady = true;
            m_LoadingPaths.erase(path);
            return true;
        }
        else return false;
        

    case AssetType::Mesh:
        std::cout << "[AssetManager-LoadAsset] Detected Mesh: " << path << std::endl;
        if(m_MeshManager.LoadMesh(path.c_str(), static_cast<Mesh*>(result.Data))){
            uint32_t iD = m_MeshManager.CreateMesh(static_cast<Mesh*>(result.Data));
            result.iD = iD;
            m_MeshManager.RegisterMesh(path.c_str(), iD);
            result.IsReady = true;
            m_LoadingPaths.erase(path);
            return true;
        }
        else return false;


    case AssetType::Material:
        std::cout << "[AssetManager-LoadAsset] Detected Material: " << path << std::endl;
        
        break;

    default:
        std::cerr << "[AssetManager-LoadAsset] Unknown file type: " << path << std::endl;
        break;
    }
    return false;
}

AssetHandle AssetManager::GetAsset(const std::string &path)
{
    AssetHandle result;
    AssetType type = GetAssetTypeFromExtension(path);
    
    if (type == AssetType::Mesh){
        result = m_MeshManager.GetMesh(path);
        std::cout << "[AssetManager-GetAsset] Detected Mesh: " << path << std::endl;
    }
    else if (type == AssetType::Texture){
        result = m_TextureManager.GetTexture(path);
        std::cout << "[AssetManager-GetAsset] Detected Texture: " << path << std::endl;
    }
    
    if (result.Data == nullptr)
    {
        if (m_LoadingPaths.find(path) != m_LoadingPaths.end())
        {
            return result;
        }

        m_LoadingPaths.insert(path);

        switch(type) {
            case AssetType::Texture:
                
                result.Data = new TextureData();
                messageQueue->Push(std::make_unique<LoadAssetMessage>(path, result));
                break;
                
            case AssetType::Mesh:
                result.Data = new Mesh();
                messageQueue->Push(std::make_unique<LoadAssetMessage>(path, result));
                break;

            default:
                m_LoadingPaths.erase(path);
                break;
        }
    }
    return result;
}

AssetHandle AssetManager::GetAsset(AssetType Type, const uint32_t iD)
{
    switch(Type){
        case AssetType::Texture:
            return m_TextureManager.GetTexture(iD);
            
            break;
        case AssetType::Mesh:
            return m_MeshManager.GetMesh(iD);
            
            break;
        case AssetType::Material:
            break;

        default:
            std::cerr << "[AssetManager-GetAsset] Unknown file type: " << std::endl;
            break;
    }
    AssetHandle result;
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

void AssetManager::CleanUp(){
    
    m_Running = false;
        
    messageQueue->Push(std::make_unique<Message>());

    if (m_WorkerThread.joinable()) {
        m_WorkerThread.join();
    }
    m_MeshManager.CleanUp();
    m_TextureManager.CleanUp();
    
}
