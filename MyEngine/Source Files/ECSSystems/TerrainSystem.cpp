//
//  TerrainSystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 21/02/2026.
//

#include "ECSSystems/TerrainSystem.h"
#include "AssetManager.h"
#include "ECS/Coordinator.h"
#include "Shader.h"
#include "stb_image.h"

#include "GLAD/include/glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



void TerrainSystem::Init(){
    
}

void TerrainSystem::CreateTerrain(Entity entity) {
    auto* terrain = m_Coordinator->GetComponent<TerrainComponent>(entity);
    if(terrain->heightmapPath.empty()) return;

    int channels;

    unsigned char* data = AssetManager::Get().GetTextureManager().LoadRawData(terrain->heightmapPath, terrain->width, terrain->height, channels);

    if (data) {
        terrain->heightData.resize(terrain->width * terrain->height);
        for (int i = 0; i < terrain->width * terrain->height; i++) {
            terrain->heightData[i] = data[i] / 255.0f;
        }
        stbi_image_free(data);
    }

    GenerateMesh(entity);
}

void TerrainSystem::Render(Shader& shader){

    for (auto const& entity : mEntities) {
        auto* terrain = m_Coordinator->GetComponent<TerrainComponent>(entity);
        auto* transform = m_Coordinator->GetComponent<TransformComponent>(entity);
        
        if(!terrain->isInitialized)
        {
            if(terrain->heightmapPath.empty()) continue;
            CreateTerrain(entity);
        }
        

        glm::mat4 model = GetWorldMatrix(transform);
        shader.SetMatrix4(model, "transformMatrix");



        bool hasAlbedo = false;
        if (terrain->mainTexturnId != UINT32_MAX)
        {
           AssetHandle albedoHandle = AssetManager::Get().GetAsset(AssetType::Texture, terrain->mainTexturnId);
           if(albedoHandle.IsReady && albedoHandle.Data)
           {
               TextureData* albedoTex = static_cast<TextureData*>(albedoHandle.Data);
               glActiveTexture(GL_TEXTURE0);
               glBindTexture(GL_TEXTURE_2D, albedoTex->TextureObject);
               glUniform1i(glGetUniformLocation(shader.shaderProgram, "mainTexture"), 0);
               hasAlbedo = true;
           }
        }
        
        shader.SetBool(hasAlbedo, "u_HasTexture");
        shader.SetBool(false, "u_HasNormalMap");
        
        
        glDisable(GL_CULL_FACE);
        
        glBindVertexArray(terrain->VAO);
        glDrawElements(GL_TRIANGLE_STRIP, terrain->indexCount, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(0);
        glEnable(GL_CULL_FACE);
    }
}

void TerrainSystem::GenerateMesh(Entity entity) {
    auto* terrain = m_Coordinator->GetComponent<TerrainComponent>(entity);
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // 1. Vertices + Normals
    for (int z = 0; z < terrain->height; z++) {
        for (int x = 0; x < terrain->width; x++) {
            float y = terrain->heightData[z * terrain->width + x] * terrain->maxHeight;
            
            Vertex v;
    
            v.position = glm::vec3(x * terrain->terrainScale, y, z * terrain->terrainScale);
            v.uv = glm::vec2((float)x / (terrain->width - 1), (float)z / (terrain->height - 1));
            
            float hL = (x > 0) ? terrain->heightData[z * terrain->width + (x - 1)] : y;
            float hR = (x < terrain->width - 1) ? terrain->heightData[z * terrain->width + (x + 1)] : y;
            float hD = (z > 0) ? terrain->heightData[(z - 1) * terrain->width + x] : y;
            float hU = (z < terrain->height - 1) ? terrain->heightData[(z + 1) * terrain->width + x] : y;

            v.normal = glm::normalize(glm::vec3(hL - hR, 2.0f, hD - hU));
            
            vertices.push_back(v);
        }
    }

    // 2. Indices logic
    for (int z = 0; z < terrain->height - 1; z++) {
        for (int x = 0; x < terrain->width; x++) {
            indices.push_back(z * terrain->width + x);
            indices.push_back((z + 1) * terrain->width + x);
        }
        if (z < terrain->height - 2) {
            indices.push_back((z + 1) * terrain->width + (terrain->width - 1));
            indices.push_back((z + 1) * terrain->width);
        }
    }
    terrain->indexCount = (unsigned int)indices.size();

    // 3. OpenGL Buffer Setup
    glGenVertexArrays(1, &terrain->VAO);
    glGenBuffers(1, &terrain->VBO);
    glGenBuffers(1, &terrain->EBO);

    glBindVertexArray(terrain->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, terrain->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // UVs
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    glBindVertexArray(0);
    terrain->isInitialized = true;
    m_TerrainEntity = entity;

}

float TerrainSystem::GetHeightAt(Entity entity, float worldX, float worldZ) {
    auto* terrain = m_Coordinator->GetComponent<TerrainComponent>(entity);
    auto* transform = m_Coordinator->GetComponent<TransformComponent>(entity);
    
    if (!terrain || terrain->heightData.empty()) return 0.0f;
    
    float localX = worldX - transform->position.x;
    float localZ = worldZ - transform->position.z;
    
    float effectiveScaleX = terrain->terrainScale * transform->scale.x;
    float effectiveScaleZ = terrain->terrainScale * transform->scale.z;
    
    float gridX = localX / effectiveScaleX;
    float gridZ = localZ / effectiveScaleZ;
    
    int x0 = (int)std::floor(gridX);
    int z0 = (int)std::floor(gridZ);
    
    if (x0 < 0 || x0 >= terrain->width - 1 || z0 < 0 || z0 >= terrain->height - 1) return 0.0f;
    
    float dx = gridX - (float)x0;
    float dz = gridZ - (float)z0;
    
    float h00 = terrain->heightData[z0 * terrain->width + x0];         // Top-Left
    float h10 = terrain->heightData[z0 * terrain->width + (x0 + 1)];   // Top-Right
    float h01 = terrain->heightData[(z0 + 1) * terrain->width + x0];   // Bottom-Left
    float h11 = terrain->heightData[(z0 + 1) * terrain->width + (x0 + 1)]; // Bottom-Right
    
    float heightTop = (1.0f - dx) * h00 + dx * h10;
    float heightBottom = (1.0f - dx) * h01 + dx * h11;
    float finalHeight = (1.0f - dz) * heightTop + dz * heightBottom;
    
    return (finalHeight * terrain->maxHeight * transform->scale.y) + transform->position.y;
}

glm::mat4 TerrainSystem::GetWorldMatrix(const TransformComponent* transform) {
    if(!transform) return;
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, transform->position);
    
    model = glm::rotate(model, glm::radians(transform->rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(transform->rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(transform->rotation.z), glm::vec3(0, 0, 1));

    model = glm::scale(model, transform->scale);
    return model;
}
