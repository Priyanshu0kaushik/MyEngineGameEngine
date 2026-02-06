//
//  MeshManager.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 18/11/2025.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include "AssetData.h"
#include "MessageQueue.h"

class MeshManager {
public:
    MeshManager();
    static void PrintMemory();
    
    bool LoadMesh(const std::string& path, Mesh* target);
    AssetHandle GetMesh(uint32_t meshID);
    AssetHandle GetMesh(const std::string& path);
    void TriangulateFace(const std::vector<uint32_t>& polygonIndices, std::vector<Face>& outFaces);

    std::unordered_map<std::string, uint32_t>& GetAllMeshes(){return m_PathToID;}
    uint32_t CreateMesh(Mesh* meshData);
    void RegisterMesh(const std::string& path, uint32_t iD);
    
    void AddReference(const std::string& path){
        auto it = m_PathToID.find(path);
        if(it == m_PathToID.end()) return;
        uint32_t iD = it->second;
        m_MeshRefCount[iD]++;
    }
    
    void RemoveReference(const std::string& path);
    void CleanUp();
private:
    
    bool SaveMeshBinary(const std::string& path, const Mesh& mesh);
    bool LoadMeshBinary(const std::string& path, Mesh& outMesh);
    void CalculateTangents(Mesh& mesh);
private:
    std::unordered_map<uint32_t, int> m_MeshRefCount;
    std::unordered_map<uint32_t, Mesh*> m_Meshes;
    uint32_t m_NextMeshID = 1;
    uint32_t m_placeHolderID = 0;
    
    std::unordered_map<std::string, uint32_t> m_PathToID;
};
