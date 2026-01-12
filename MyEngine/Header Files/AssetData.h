//
//  AssetTypes.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 04/01/2026.
//

#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>


enum class AssetType{
    None,
    Texture,
    Material,
    Mesh,
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Face {
    std::vector<uint32_t> vertexIndices;
};

struct BaseData{
    virtual ~BaseData() = default;
    bool IsLoaded = false;
    AssetType Type = AssetType::None;
};


struct Mesh : public BaseData{
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
};

struct TextureData : public BaseData{
    int Width;
    int Height;
    
    unsigned int TextureObject;
};

struct Material{
    glm::vec3 Ambient = { 0.1f, 0.1f, 0.1f };
    glm::vec3 Diffuse = { 1.0f, 1.0f, 1.0f };
    glm::vec3 Specular = { 0.5f, 0.5f, 0.5f };
    float Shininess = 32.0f;

    uint32_t albedoID = UINT32_MAX;
    uint32_t normalID  = UINT32_MAX;
    uint32_t specID    = UINT32_MAX;

    std::string albedoPath = "";
    std::string normalPath  = "";
    std::string specPath    = "";
};

struct AssetHandle{
    BaseData* Data;
    uint32_t iD = UINT32_MAX;
    bool IsReady = false;
};
