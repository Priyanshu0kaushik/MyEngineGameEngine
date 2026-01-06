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

struct AssetHandle{
    BaseData* Data;
    bool IsReady = false;
};
