//
//  MaterialManager.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 10/01/2026.
//

#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include "MessageQueue.h"
#include "AssetData.h"

//class MaterialManager
//{
//public:
//    MaterialManager();    
//    bool LoadTexture(const std::string& path, Material* target);
//    AssetHandle GetMaterial(uint32_t materialId);
//    AssetHandle GetMaterial(const std::string& path);
//    std::unordered_map<std::string, uint32_t>& GetAllMaterials(){return m_PathToID;}
//    
//    void RegisterMaterial(const std::string& path, uint32_t iD);
//    uint32_t CreateMaterial(Material* materialData);
//    
//private:
//
//    std::unordered_map<uint32_t, Material*> m_Material;
//    uint32_t m_NextMaterialID = 1;
//    uint32_t m_placeHolderID = 1;
//    std::unordered_map<std::string, uint32_t> m_PathToID;
//
//};
