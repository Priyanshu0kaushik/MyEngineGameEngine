//
//  Message.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 08/12/2025.
//

#pragma once
#include <iostream>
#include "AssetData.h"

enum class MessageType {
   
    LoadAsset,
    AssetLoaded
};
struct Message{
    MessageType type;
    virtual ~Message() = default;
};

struct LoadAssetMessage : public Message {
    std::string path;
    AssetHandle assetHandle;
    LoadAssetMessage(const std::string& p, AssetHandle& handle) : path(p), assetHandle(handle) { type = MessageType::LoadAsset; }
};

struct AssetLoadedMessage : public Message {
    AssetType assetType;
    std::string path;
    
    AssetLoadedMessage(std::string _path, AssetType _type)
        : path(_path), assetType(_type) { type = MessageType::AssetLoaded; }
};

