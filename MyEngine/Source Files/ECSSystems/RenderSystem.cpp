//
//  RenderSystem.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 19/11/2025.
//

#include "ECSSystems/RenderSystem.h"
#include "ECS/Coordinator.h"
#include "AssetManager.h"
#include "GLAD/include/glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void RenderSystem::Init()
{
}

// MESH UPLOAD TO GPU
void RenderSystem::UploadMeshIfNeeded(Entity e, MeshComponent* mc)
{
    // Checks
    if (mc->meshID == UINT32_MAX) return;
    AssetHandle handle = AssetManager::Get().GetAsset(AssetType::Mesh, mc->meshID);
    
    if(!handle.IsReady || handle.Data == nullptr) return;
    if(handle.Data->Type != AssetType::Mesh) return;
    if(!handle.Data->IsLoaded) return;
    
    Mesh* mesh = static_cast<Mesh*>(handle.Data);
    if(mesh==nullptr) return;
    
    // If already on GPU, return.
    if(mesh->uploaded) return;
    
    
    // DATA
    // Layout: [PosX, PosY, PosZ, NormX, NormY, NormZ, U, V, TanX, TanY, TanZ]
    // Stride: 11 floats
    
    std::vector<float> gpuVertices;
    std::vector<uint32_t> gpuIndices;
    gpuVertices.reserve(mesh->vertices.size() * 11);
    gpuIndices.reserve(mesh->faces.size() * 3);

    for (const auto& v : mesh->vertices)
    {
        // Position
        gpuVertices.push_back(v.position.x);
        gpuVertices.push_back(v.position.y);
        gpuVertices.push_back(v.position.z);

        // Normal
        gpuVertices.push_back(v.normal.x);
        gpuVertices.push_back(v.normal.y);
        gpuVertices.push_back(v.normal.z);

        // UV
        gpuVertices.push_back(v.uv.x);
        gpuVertices.push_back(v.uv.y);
        
        // Tangent
        gpuVertices.push_back(v.tangent.x);
        gpuVertices.push_back(v.tangent.y);
        gpuVertices.push_back(v.tangent.z);
    }

    // Flatten face indices for EBO
    for (const auto& face : mesh->faces)
    {
        for (int index : face.vertexIndices)
            gpuIndices.push_back(index);
    }

    mesh->indexCount = static_cast<int>(gpuIndices.size());
    int verticesSize = static_cast<int>(gpuVertices.size());

    std::cout<<"[GPU Upload] Mesh Uploaded: " << mesh->indexCount << " Indices || " << verticesSize << " Vertices" << std::endl;
    
    // Generate OpenGL Buffers
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    // Upload Vertex Data
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, gpuVertices.size() * sizeof(float), gpuVertices.data(), GL_STATIC_DRAW);

    // Upload Index Data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gpuIndices.size() * sizeof(uint32_t), gpuIndices.data(), GL_STATIC_DRAW);

    int stride = 11 * sizeof(float);
    
    // 1. Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    // 2. Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

    // 3. UV
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    
    // 4. Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    mesh->uploaded = true;
}

glm::mat4 RenderSystem::BuildModelMatrix(TransformComponent* t)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, t->position);
    model = glm::rotate(model, glm::radians(t->rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(t->rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(t->rotation.z), glm::vec3(0, 0, 1));
    
    model = glm::scale(model, t->scale);
    return model;
}

// RENDER LOOP
// Iterates over all entities with a Mesh + Transform and draw.

void RenderSystem::Render(Shader& shader)
{
    for (Entity e : mEntities)
    {
        TransformComponent* transform = m_Coordinator->GetComponent<TransformComponent>(e);
        MeshComponent* meshComp  = m_Coordinator->GetComponent<MeshComponent>(e);

        UploadMeshIfNeeded(e, meshComp);

        glm::mat4 model = BuildModelMatrix(transform);
        shader.SetMatrix4(model, "transformMatrix");
        
        // Upload Material
        shader.SetVec3("u_Material.ambient", meshComp->material.Ambient);
        shader.SetVec3("u_Material.diffuse", meshComp->material.Diffuse);
        shader.SetVec3("u_Material.specular", meshComp->material.Specular);
        shader.SetFloat("u_Material.shininess", meshComp->material.Shininess);
        
        bool hasAlbedo = false;
        bool hasSpec = false;
        bool hasNormal = false;
        
        
        // TEXTURE BINDING
        
        // 1. Albedo Map -> Unit 0
        if (meshComp->material.albedoID != UINT32_MAX)
        {
           AssetHandle albedoHandle = AssetManager::Get().GetAsset(AssetType::Texture, meshComp->material.albedoID);
           if(albedoHandle.IsReady && albedoHandle.Data)
           {
               TextureData* albedoTex = static_cast<TextureData*>(albedoHandle.Data);
               glActiveTexture(GL_TEXTURE0);
               glBindTexture(GL_TEXTURE_2D, albedoTex->TextureObject);
               glUniform1i(glGetUniformLocation(shader.shaderProgram, "mainTexture"), 0);
               hasAlbedo = true;
           }
        }
        
        // 2. Normal Map -> Unit 1
        if (meshComp->material.normalID != UINT32_MAX)
        {
           AssetHandle normalHandle = AssetManager::Get().GetAsset(AssetType::Texture, meshComp->material.normalID);
           if(normalHandle.IsReady && normalHandle.Data)
           {
               TextureData* normalTex = static_cast<TextureData*>(normalHandle.Data);
               glActiveTexture(GL_TEXTURE1);
               glBindTexture(GL_TEXTURE_2D, normalTex->TextureObject);
               glUniform1i(glGetUniformLocation(shader.shaderProgram, "normalMap"), 1);
               hasNormal = true;
           }
        }
        
        // 3. Specular Map -> Unit 2
        if (meshComp->material.specID != UINT32_MAX)
        {
           AssetHandle specHandle = AssetManager::Get().GetAsset(AssetType::Texture, meshComp->material.specID);
           if(specHandle.IsReady && specHandle.Data)
           {
               TextureData* specTex = static_cast<TextureData*>(specHandle.Data);
               glActiveTexture(GL_TEXTURE2);
               glBindTexture(GL_TEXTURE_2D, specTex->TextureObject);
               glUniform1i(glGetUniformLocation(shader.shaderProgram, "specularMap"), 2);
               hasSpec = true;
           }
        }
        
        // Set Shader Flags so GPU knows what textures do we have
        shader.SetBool(hasSpec, "u_HasSpecularMap");
        shader.SetBool(hasAlbedo, "u_HasTexture");
        shader.SetBool(hasNormal, "u_HasNormalMap");
        
        // DRAW
        Mesh* mesh = static_cast<Mesh*>(AssetManager::Get().GetAsset(AssetType::Mesh, meshComp->meshID).Data);
        if (mesh && mesh->uploaded)
        {
            glBindVertexArray(mesh->VAO);
            glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    glBindVertexArray(0);
}

