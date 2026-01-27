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

void RenderSystem::UploadMeshIfNeeded(Entity e, MeshComponent* mc)
{
    if (mc->uploaded) return;
    
    AssetHandle handle = AssetManager::Get().GetMeshManager().GetMesh(mc->meshID);
    if(!handle.IsReady || !handle.Data) return;
    if(handle.Data->Type != AssetType::Mesh) return;
    Mesh* mesh = static_cast<Mesh*>(handle.Data);
    if(mesh==nullptr) return;
    std::vector<float> gpuVertices;
    std::vector<uint32_t> gpuIndices;
    gpuVertices.reserve(mesh->vertices.size() * 8);
    gpuIndices.reserve(mesh->faces.size() * 3);

    for (const auto& v : mesh->vertices)
    {
        gpuVertices.push_back(v.position.x);
        gpuVertices.push_back(v.position.y);
        gpuVertices.push_back(v.position.z);

        gpuVertices.push_back(v.normal.x);
        gpuVertices.push_back(v.normal.y);
        gpuVertices.push_back(v.normal.z);

        gpuVertices.push_back(v.uv.x);
        gpuVertices.push_back(v.uv.y);
    }

    for (const auto& face : mesh->faces)
    {
        for (int index : face.vertexIndices)
            gpuIndices.push_back(index);
    }

    mc->indexCount = static_cast<int>(gpuIndices.size());
    int verticesSize = static_cast<int>(gpuVertices.size());

    std::cout<<"Indices + vertices : "<<mc->indexCount + verticesSize <<std::endl;
    glGenVertexArrays(1, &mc->VAO);
    glGenBuffers(1, &mc->VBO);
    glGenBuffers(1, &mc->EBO);

    glBindVertexArray(mc->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mc->VBO);
    glBufferData(GL_ARRAY_BUFFER, gpuVertices.size() * sizeof(float), gpuVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mc->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gpuIndices.size() * sizeof(uint32_t), gpuIndices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    mc->uploaded = true;
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

void RenderSystem::Render(Shader& shader)
{
    for (Entity e : mEntities)
    {
        TransformComponent* transform = m_Coordinator->GetComponent<TransformComponent>(e);
        MeshComponent* meshComp  = m_Coordinator->GetComponent<MeshComponent>(e);

        UploadMeshIfNeeded(e, meshComp);

        glm::mat4 model = BuildModelMatrix(transform);
        shader.SetMatrix4(model, "transformMatrix");
        // set material settings
        shader.SetVec3("u_Material.ambient", meshComp->material.Ambient);
        shader.SetVec3("u_Material.diffuse", meshComp->material.Diffuse);
        shader.SetVec3("u_Material.specular", meshComp->material.Specular);
        shader.SetFloat("u_Material.shininess", meshComp->material.Shininess);
        
        bool hasAlbedo = false;
        bool hasSpec = false;
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
        if (meshComp->material.specID != UINT32_MAX)
        {
           AssetHandle specHandle = AssetManager::Get().GetAsset(AssetType::Texture, meshComp->material.specID);
           if(specHandle.IsReady && specHandle.Data)
           {
               TextureData* specTex = static_cast<TextureData*>(specHandle.Data);
               glActiveTexture(GL_TEXTURE1);
               glBindTexture(GL_TEXTURE_2D, specTex->TextureObject);
               glUniform1i(glGetUniformLocation(shader.shaderProgram, "specularMap"), 1);
               hasSpec = true;
           }
        }
        
        shader.SetBool(hasSpec, "u_HasSpecularMap");
        shader.SetBool(hasAlbedo, "u_HasTexture");
        
        glBindVertexArray(meshComp->VAO);
        glDrawElements(GL_TRIANGLES, meshComp->indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }

    glBindVertexArray(0);
}

