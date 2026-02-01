//
//  TextureManager.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 02/12/2025.
//

#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include "MessageQueue.h"
#include "AssetData.h"

class TextureManager
{
public:
    TextureManager();    
    bool LoadTexture(const std::string& path, TextureData* target);
    AssetHandle GetTexture(uint32_t textureId);
    AssetHandle GetTexture(const std::string& path);
    std::unordered_map<std::string, uint32_t>& GetAllTextures(){return m_PathToID;}
    
    void RegisterTexture(const std::string& path, uint32_t iD);
    uint32_t CreateTexture(TextureData* textureData);
    
    void SetMipMapSettings(uint32_t textureId, int mode);
    
    void CleanUp();
private:

    std::unordered_map<uint32_t, TextureData*> m_Textures;
    uint32_t m_NextTextureID = 1;
    uint32_t m_placeHolderID = 1;
    std::unordered_map<std::string, uint32_t> m_PathToID;

};
