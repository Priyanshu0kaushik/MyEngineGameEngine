//
//  TextureManager.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 02/12/2025.
//

#include "TextureManager.h"
#include <iostream>
#include "glad.h"
#include "glfw3.h"
#include "stb_image.h"
#include <assert.h>

TextureManager::TextureManager(){
    
}

bool TextureManager::LoadTexture(const std::string& path, TextureData* target){
    if (m_PathToID.find(path) != m_PathToID.end())
        return true;
    
    
    TextureData Texture;
    stbi_set_flip_vertically_on_load(true);
    int Channels = 0;
    unsigned char *data = stbi_load(path.c_str(), &Texture.Width, &Texture.Height, &Channels, 0);

    glGenTextures(1, &Texture.TextureObject);
    glBindTexture(GL_TEXTURE_2D, Texture.TextureObject);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    GLenum format = GL_RGB;
    if (Channels == 4)
        format = GL_RGBA;
    
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Texture.Width, Texture.Height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        return false;
    }
    stbi_image_free(data);
    
    *target = Texture;
    target->IsLoaded = true;
    return true;
}

uint32_t TextureManager::CreateTexture(TextureData* textureData)
{
    uint32_t id = m_NextTextureID++;
    m_Textures[id] = textureData;
    return id;
}

AssetHandle TextureManager::GetTexture(uint32_t textureId){
    AssetHandle result;
    if(textureId==UINT32_MAX){
        result.Data = nullptr;
    }
    auto it = m_Textures.find(textureId);
    
    if (it != m_Textures.end()) result.Data = it->second;
    
    else{
        auto it1 = m_Textures.find(m_placeHolderID);
        if(it1!= m_Textures.end()) result.Data = it1->second;
        else result.Data = nullptr;
    }
    if(result.Data) result.Data->Type = AssetType::Texture;
    result.IsReady = true;
    return result;
}

AssetHandle TextureManager::GetTexture(const std::string& path){
    auto it = m_PathToID.find(path);
    if(it != m_PathToID.end()) return GetTexture(it->second);
    else return GetTexture(UINT32_MAX);
}

void TextureManager::RegisterTexture(const std::string &path, uint32_t iD){
    m_PathToID[path] = iD;
}
