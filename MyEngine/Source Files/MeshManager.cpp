//
//  MeshManager.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 18/11/2025.
//

#include "MeshManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/sysctl.h>
#include <mach/mach.h>

MeshManager::MeshManager()
{
    
}

bool MeshManager::LoadMesh(const std::string& path, Mesh* target)
{
    PrintMemory();
    if (m_PathToID.find(path) != m_PathToID.end())
        return true;

    std::string binPath = path + ".memesh";
    Mesh meshData;

    if (std::filesystem::exists(binPath))
    {
        std::cout<<"BinaryLoading"<< std::endl;
        LoadMeshBinary(binPath, meshData);
        *target = meshData;
        return true;
    }
    else{
        std::vector<glm::vec3> tempPositions;
        std::vector<glm::vec2> tempUVs;
        std::vector<glm::vec3> tempNormals;
        
        std::ifstream file(path);
        if (!file.is_open())
        {
            std::cout << "Failed to load mesh: " << path << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;
            
            if (prefix == "v")
            {
                glm::vec3 pos;
                ss >> pos.x >> pos.y >> pos.z;
                tempPositions.push_back(pos);
            }
            else if (prefix == "vt")
            {
                glm::vec2 uv;
                ss >> uv.x >> uv.y;
                tempUVs.push_back(uv);
            }
            else if (prefix == "vn")
            {
                glm::vec3 normal;
                ss >> normal.x >> normal.y >> normal.z;
                tempNormals.push_back(normal);
            }

            else if (prefix == "f")
            {
                std::vector<uint32_t> polygonIndices;
                std::string vertexData;
                std::unordered_map<std::string, int> uniqueVertices;

                while (ss >> vertexData)
                {
                    if (uniqueVertices.count(vertexData) > 0)
                    {
                        polygonIndices.push_back(uniqueVertices[vertexData]);
                    }
                    else
                    {
                        std::stringstream vs(vertexData);
                        std::string vStr, tStr, nStr;

                        std::getline(vs, vStr, '/');
                        std::getline(vs, tStr, '/');
                        std::getline(vs, nStr, '/');

                        int vIndex = std::stoi(vStr) - 1;
                        int tIndex = tStr.empty() ? -1 : std::stoi(tStr) - 1;
                        int nIndex = nStr.empty() ? -1 : std::stoi(nStr) - 1;

                        Vertex vert;
                        vert.position = tempPositions[vIndex];
                        vert.uv = (tIndex >= 0 ? tempUVs[tIndex] : glm::vec2(0.0f));
                        vert.normal = (nIndex >= 0 ? tempNormals[nIndex] : glm::vec3(0, 1, 0));

                        meshData.vertices.push_back(vert);

                        polygonIndices.push_back((int)meshData.vertices.size() - 1);
                    }
                }

                if (polygonIndices.size() == 3)
                {
                    Face face;
                    face.vertexIndices = polygonIndices;
                    meshData.faces.push_back(face);
                }
                else if (polygonIndices.size() > 3)
                {
                    TriangulateFace(polygonIndices, meshData.faces);
                    
                }
            }
        }

        file.close();
        std::string binPath = path + ".memesh";
        SaveMeshBinary(binPath, meshData);
    }

    *target = meshData;
    target->IsLoaded = true;
    return true;
}

uint32_t MeshManager::CreateMesh(Mesh* meshData)
{
    uint32_t id = m_NextMeshID++;
    m_Meshes[id] = meshData;
    return id;
}

void MeshManager::RegisterMesh(const std::string &path, uint32_t iD){
    m_PathToID[path] = iD;
}

AssetHandle MeshManager::GetMesh(uint32_t meshID)
{
    AssetHandle result;
    if(meshID==UINT32_MAX){
        result.Data = nullptr;
        return result;
    }
    else{
        auto it = m_Meshes.find(meshID);
        
        if (it != m_Meshes.end()) result.Data = it->second;
        
        else{
            auto it1 = m_Meshes.find(m_placeHolderID);
            if(it1!= m_Meshes.end()) result.Data = it1->second;
            else result.Data = nullptr;
        }
    }
    
    if(result.Data) result.Data->Type = AssetType::Mesh;
    result.IsReady = true;
    return result;
}

AssetHandle MeshManager::GetMesh(const std::string &path){
    auto it = m_PathToID.find(path);
    if(it != m_PathToID.end()) return GetMesh(it->second);
    else return GetMesh(UINT32_MAX);
}

void MeshManager::TriangulateFace(const std::vector<uint32_t> &polygonIndices, std::vector<Face> &outFaces){
    if (polygonIndices.size() < 3)
        return;

    for (size_t i = 1; i + 1 < polygonIndices.size(); ++i)
    {
        Face face;
        face.vertexIndices.push_back(polygonIndices[0]);
        face.vertexIndices.push_back(polygonIndices[i]);
        face.vertexIndices.push_back(polygonIndices[i + 1]);
        outFaces.push_back(face);
    }
}

void MeshManager::PrintMemory()
{
    int mib [] = { CTL_HW, HW_MEMSIZE };
    uint64_t totalMem = 0;
    size_t len = sizeof(totalMem);
    
    if(-1 == sysctl(mib, 2, &totalMem, &len, NULL, 0))
    {
        std::cerr << "Failed to get total memory.\n";
        return;
    }
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmstat;

    if (host_statistics64(mach_host_self(),
                          HOST_VM_INFO64,
                          (host_info64_t)&vmstat,
                          &count) != KERN_SUCCESS)
    {
        std::cerr << "Failed to get VM statistics.\n";
        return;
    }
    
    uint64_t pageSize = 0;
    len = sizeof(pageSize);
    sysctlbyname("hw.pagesize", &pageSize, &len, NULL, 0);

    uint64_t freeMem  = (vmstat.free_count + vmstat.inactive_count) * pageSize;
    uint64_t usedMem  = totalMem - freeMem;

//    uint64_t virtualMemUsed = vmstat.internal_page_count * pageSize;
    uint64_t virtualMemFree = vmstat.free_count * pageSize;

    std::cout << "There is "
              << freeMem / (1024 * 1024)
              << " MB of physical memory available.\n";

    std::cout << "There is "
              << virtualMemFree / (1024 * 1024)
              << " MB of virtual memory free.\n";

    std::cout << "Total physical memory: "
              << totalMem / (1024 * 1024)
              << " MB\n";

    std::cout << "Used physical memory: "
              << usedMem / (1024 * 1024)
              << " MB\n";
}

bool MeshManager::SaveMeshBinary(const std::string &path, const Mesh &mesh)
{
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open())
    {
        std::cout << "Failed to save mesh: " << path << std::endl;
        return false;
    }

    uint32_t vertexCount = (uint32_t)mesh.vertices.size();
    out.write((char*)&vertexCount, sizeof(uint32_t));
    
    out.write((char*)mesh.vertices.data(), vertexCount * sizeof(Vertex));

    
    uint32_t faceCount = (uint32_t)mesh.faces.size();
    out.write((char*)&faceCount, sizeof(uint32_t));

    
    for (const Face& face : mesh.faces)
    {
        out.write((char*)face.vertexIndices.data(), 3 * sizeof(int));
    }

    out.close();
    
    return true;
}

bool MeshManager::LoadMeshBinary(const std::string &path, Mesh &outMesh)
{
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open())
    {
        std::cout << "Failed to load mesh binary: " << path << std::endl;
        return false;
    }
    
    outMesh.vertices.clear();
    outMesh.faces.clear();

    uint32_t vertexCount = 0;
    in.read((char*)&vertexCount, sizeof(uint32_t));

    outMesh.vertices.resize(vertexCount);
    in.read((char*)outMesh.vertices.data(),
            vertexCount * sizeof(Vertex));

    uint32_t faceCount = 0;
    in.read((char*)&faceCount, sizeof(uint32_t));

    for (uint32_t i = 0; i < faceCount; i++)
    {
        Face face;

        face.vertexIndices.resize(3);
        in.read((char*)face.vertexIndices.data(), 3 * sizeof(int));
        
        outMesh.faces.push_back(face);
    }

    in.close();
    return true;
}

