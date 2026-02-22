//
//  TextureManager.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 02/12/2025.
//

#include "TextureManager.h"
#include <iostream>
#include "GLAD/include/glad/glad.h"
#include "glfw3.h"
#include "stb_image.h"
#include <assert.h>

TextureManager::TextureManager(){
    
}

bool TextureManager::LoadTexture(const std::string& path, TextureData* target){
    if (!target) return false;

    stbi_set_flip_vertically_on_load(true);
    int Channels = 0;
    unsigned char *data = stbi_load(path.c_str(), &target->Width, &target->Height, &Channels, 0);

    if (data) {
        glGenTextures(1, &target->TextureObject);
        glBindTexture(GL_TEXTURE_2D, target->TextureObject);
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        GLenum format = (Channels == 4) ? GL_RGBA : GL_RGB;
    
        glTexImage2D(GL_TEXTURE_2D, 0, format, target->Width, target->Height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        stbi_image_free(data);
        target->IsLoaded = true;
        return true;
    }
    
    std::cout << "Failed to load: " << path << std::endl;
    return false;
}

uint32_t TextureManager::CreateTexture(TextureData* textureData)
{
    uint32_t id = m_NextTextureID++;
    m_Textures[id] = textureData;
    return id;
}

unsigned char* TextureManager::LoadRawData(const std::string& path, int& width, int& height, int& channels) {
    stbi_set_flip_vertically_on_load(true);
    return stbi_load(path.c_str(), &width, &height, &channels, 1);
}

AssetHandle TextureManager::GetTexture(uint32_t textureId){
    AssetHandle result;
    if(textureId==UINT32_MAX){
        result.Data = nullptr;
        return result;
    }
    auto it = m_Textures.find(textureId);
    
    if (it != m_Textures.end()) result.Data = it->second;
    
    else{
        auto it1 = m_Textures.find(m_placeHolderID);
        if(it1!= m_Textures.end()) result.Data = it1->second;
        else result.Data = nullptr;
    }
    if(result.Data) result.Data->Type = AssetType::Texture;
    result.iD = textureId;
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

void TextureManager::SetMipMapSettings(uint32_t textureId, int mode) {
    auto it = m_Textures.find(textureId);
    if (it != m_Textures.end() && it->second->IsLoaded) {
        glBindTexture(GL_TEXTURE_2D, it->second->TextureObject);
        
        if (mode == 0) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        } else if (mode == 1) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else if (mode == 2) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
    }
}

void TextureManager::CleanUp(){
    for (auto& [id, data] : m_Textures) {
        if (data) {
            glDeleteTextures(1, &data->TextureObject);

            delete data;
        }
    }
    m_Textures.clear();
    m_TextureRefCount.clear();
    m_PathToID.clear();

}

void TextureManager::RemoveReference(const std::string &path) {
    auto it = m_PathToID.find(path);
    uint32_t iD = it != m_PathToID.end()? it->second : UINT32_MAX;
    if(iD == UINT32_MAX) return;
    
    if (--m_TextureRefCount[iD] > 0) return;
    
    TextureData* data = m_Textures[iD];
    
    if (data)
    {
        if (data->TextureObject != 0) {
            glDeleteTextures(1, &data->TextureObject);
        }
        delete data;
    }
    std::cout<<"Texture Unloaded"<<std::endl;
    m_Textures.erase(iD);
    m_TextureRefCount.erase(iD);
    m_PathToID.erase(path);
    
}

