//
//  Scene.cpp
//  MyEngine
//
//  Created by Priyanshu Kaushik on 30/10/2025.
//
#include "Scene.h"
#include "Shader.h"
#include "Components.h"
#include "ECSSystems/RenderSystem.h"
#include "AssetManager.h"
#include "Project.h"
#include <fstream>
#include <sstream>


Entity Scene::AddEntity(char* aName)
{
    Entity e = m_Coordinator.CreateEntity();

//     Add transform component
    TransformComponent transform{};
    transform.position = {0.0f, 0.0f, 0.0f};
    transform.rotation = {0.0f, 0.0f, 0.0f};
    transform.scale = {1.0f, 1.0f, 1.0f};
    m_Coordinator.AddComponent(e, transform);

    RenameEntity(e, aName);
    return e;
}

void Scene::Save() {
    const std::string& filePath = Project::GetActiveAbsoluteScenePath();
    std::ofstream file(filePath);
    if (!file.is_open()) return;

    auto allEntities = m_Coordinator.GetAliveEntities();

    for (auto entity : allEntities) {
        auto* nc = m_Coordinator.GetComponent<NameComponent>(entity);
        if (nc && nc->Name == "Editor Camera") {
            continue;
        }
        file << "[Entity]\n";

        // 1. Name Component
        if (auto* nc = m_Coordinator.GetComponent<NameComponent>(entity)) {
            file << "Name: " << nc->Name << "\n";
        }

        // 2. Transform Component
        if (auto* tc = m_Coordinator.GetComponent<TransformComponent>(entity)) {
            file << "Transform: "
                 << tc->position.x << " " << tc->position.y << " " << tc->position.z << " | "
                 << tc->rotation.x << " " << tc->rotation.y << " " << tc->rotation.z << " | "
                 << tc->scale.x << " " << tc->scale.y << " " << tc->scale.z << "\n";
        }

        
        // 3. Light Component
        if (auto* lc = m_Coordinator.GetComponent<LightComponent>(entity)) {
            file << "Light: " << (int)lc->type << " "
                 << lc->color.r << " " << lc->color.g << " " << lc->color.b << " "
                 << lc->intensity << " "
                 << lc->constant << " " << lc->linear << " " << lc->quadratic << "\n";
        }
        
        // 4. Camera Component
        if (auto* cc = m_Coordinator.GetComponent<CameraComponent>(entity)) {
            file << "Component: Camera\n";
            file << "Cam_Settings: " << cc->Fov << " " << cc->Near << " " << cc->Far << " " << cc->AspectRatio << "\n";
            file << "Cam_Orientation: " << cc->Yaw << " " << cc->Pitch << "\n";
            file << "Cam_State: " << (cc->IsPrimary ? "1" : "0") << "\n";
        }
        
        // 5. RigidBody Component
        if (auto* rb = m_Coordinator.GetComponent<RigidBodyComponent>(entity)) {
            file << "RigidBody: " << rb->mass << " " << rb->gravityScale << " " << (rb->isStatic? "1" : "0") << " " << (rb->isKinematic? "1" : "0") << "\n";
        }
        
        // 6. Collider Component
        if(auto* collider = m_Coordinator.GetComponent<ColliderComponent>(entity))
        {
            // Type, Center (x,y,z), Bounciness, Friction, isTrigger
            file << "Collider: " << (int)collider->type << " "
                 << collider->center.x << " " << collider->center.y << " " << collider->center.z << " "
                 << collider->bounciness << " " << collider->friction << " "
                 << (collider->isTrigger ? 1 : 0) << "\n";
        }

        // 7. Box Collider Component
        if(auto* boxCollider = m_Coordinator.GetComponent<BoxColliderComponent>(entity))
        {
            // Extents (x,y,z)
            file << "BoxCollider: " << boxCollider->extents.x << " "
                 << boxCollider->extents.y << " " << boxCollider->extents.z << "\n";
        }
        
        // 8. Sphere Collider Component
        if(auto* sphereCollider = m_Coordinator.GetComponent<SphereColliderComponent>(entity))
        {
            // radius
            file << "SphereCollider: " << sphereCollider->radius << "\n";
        }
        
        // 9. Mesh Component -- save at last
        if (auto* mc = m_Coordinator.GetComponent<MeshComponent>(entity)) {
            if(mc->meshPath.empty()){
                file << "MeshPath: " << "No Path" << "\n";
                continue;
            }
            else file << "MeshPath: " << mc->meshPath << "\n";
            
            // Material Data
            auto& mat = mc->material;
            file << "Mat_Ambient: " << mat.Ambient.x << " " << mat.Ambient.y << " " << mat.Ambient.z << "\n";
            file << "Mat_Diffuse: " << mat.Diffuse.x << " " << mat.Diffuse.y << " " << mat.Diffuse.z << "\n";
            file << "Mat_Shininess: " << mat.Shininess << "\n";
            
            // Texture Paths
            if (!mat.albedoPath.empty()) file << "Tex_Albedo: " << mat.albedoPath << "\n";
            if (!mat.normalPath.empty()) file << "Tex_Normal: " << mat.normalPath << "\n";
            if (!mat.specPath.empty())   file << "Tex_Spec: " << mat.specPath << "\n";
        }


        file << "[EndEntity]\n\n";
    }
    file.close();
    std::cout << "Scene saved to: " << filePath << std::endl;
}

void Scene::Load(const std::string& filePath) {
    std::cout<<"Loading Scene.."<<filePath<<std::endl;
    
    if (!std::filesystem::exists(filePath)) {
        std::cout << "ERROR: File does not exist at this path!" << std::endl;
        return;
    }
    std::ifstream file(filePath);
    if (!file.is_open()) return;
    
    std::cout<<"File opened"<<std::endl;
    
    auto entities = m_Coordinator.GetAliveEntities();
    for (auto e : entities) {
        m_Coordinator.DestroyEntity(e);
    }
    
    mPendingMeshEntities.clear();
    std::string line;
    Entity currentEntity = 0;
    bool entityCreated = false;

    while (std::getline(file, line)) {
        if (line == "[Entity]") {
            currentEntity = m_Coordinator.CreateEntity();
            entityCreated = true;
            continue;
        }
        if (!entityCreated) continue;

        // 1. Name Component
        if (line.find("Name: ") == 0) {
            NameComponent* nC = m_Coordinator.GetComponent<NameComponent>(currentEntity);
            if(nC)
            {
                nC->Name = line.substr(6);
            }
        }
        // 2. Transform Component
        else if (line.find("Transform: ") == 0) {
            TransformComponent tc;
            std::string data = line.substr(11);
            std::replace(data.begin(), data.end(), '|', ' ');
            std::stringstream ss(data);
            ss >> tc.position.x >> tc.position.y >> tc.position.z
               >> tc.rotation.x >> tc.rotation.y >> tc.rotation.z
               >> tc.scale.x >> tc.scale.y >> tc.scale.z;
            m_Coordinator.AddComponent<TransformComponent>(currentEntity, tc);
        }


        // 3. Light Component
        else if (line.find("Light: ") == 0) {
            LightComponent lc;
            std::stringstream ss(line.substr(7));
            int typeInt;
            ss >> typeInt >> lc.color.r >> lc.color.g >> lc.color.b >> lc.intensity
               >> lc.constant >> lc.linear >> lc.quadratic;
            lc.type = (LightType)typeInt;
            m_Coordinator.AddComponent<LightComponent>(currentEntity, lc);
        }
        
        // 4. Camera Component
        else if (line.find("Component: Camera") == 0) {
            CameraComponent cc;
            std::string camLine;
            while (std::getline(file, camLine) && camLine != "[EndEntity]") {
                std::stringstream ss(camLine);
                std::string tag;
                ss >> tag;
                if (tag == "Cam_Settings:") ss >> cc.Fov >> cc.Near >> cc.Far >> cc.AspectRatio;
                else if (tag == "Cam_Orientation:") ss >> cc.Yaw >> cc.Pitch;
                else if (tag == "Cam_State:") ss >> cc.IsPrimary;
                if (file.peek() == '[') break;
            }
            m_Coordinator.AddComponent<CameraComponent>(currentEntity, cc);
        }
        
        // 5. RigidBody Component
        else if (line.find("RigidBody: ") == 0)
        {
            std::cout<<"RigidBody\n";
            RigidBodyComponent rb;
            std::stringstream ss(line.substr(11));
            ss >> rb.mass >> rb.gravityScale >> rb.isStatic >> rb.isKinematic;
            
            m_Coordinator.AddComponent<RigidBodyComponent>(currentEntity, rb);
        }
        
        // 6. Collider
        else if (line.find("Collider: ") == 0) {
            std::cout<<"Collider\n";
            ColliderComponent col;
            int typeInt, triggerInt;
            std::stringstream ss(line.substr(10));
            
            ss >> typeInt >> col.center.x >> col.center.y >> col.center.z
                 >> col.bounciness >> col.friction >> triggerInt;
            
            col.type = (ColliderType)typeInt;
            col.isTrigger = (triggerInt == 1);
            m_Coordinator.AddComponent<ColliderComponent>(currentEntity, col);
        }
        
        // 7. Box Collider
        else if (line.find("BoxCollider: ") == 0) {
            std::cout<<"BoxCollider\n";
            BoxColliderComponent box;
            std::stringstream ss(line.substr(13));
            
            ss >> box.extents.x >> box.extents.y >> box.extents.z;
            m_Coordinator.AddComponent<BoxColliderComponent>(currentEntity, box);
        }
        
        // 8. Sphere Collider
        else if (line.find("SphereCollider: ") == 0) {
            std::cout<<"SphereCollider\n";
            SphereColliderComponent sphere;
            std::stringstream ss(line.substr(16));
            
            ss >> sphere.radius;
            m_Coordinator.AddComponent<SphereColliderComponent>(currentEntity, sphere);
        }
        
        // 9. Mesh Component
        else if (line.find("MeshPath: ") == 0) {
            MeshComponent mc;
            mc.meshPath = line.substr(10);
            if(mc.meshPath == "No Path") {
                mc.meshPath.clear();
                m_Coordinator.AddComponent<MeshComponent>(currentEntity, mc);
                continue;
            }
            
            std::string matLine;
            while (std::getline(file, matLine) && matLine != "[EndEntity]" && matLine != "")
            {
                std::stringstream ss(matLine);
                std::string tag; ss >> tag;
                
                if (tag == "Mat_Ambient:") ss >> mc.material.Ambient.x >> mc.material.Ambient.y >> mc.material.Ambient.z;
                else if (tag == "Mat_Diffuse:") ss >> mc.material.Diffuse.x >> mc.material.Diffuse.y >> mc.material.Diffuse.z;
                else if (tag == "Mat_Shininess:") ss >> mc.material.Shininess;
                else if (tag == "Tex_Albedo:") {
                    mc.material.albedoPath = matLine.substr(12);
                    AssetManager::Get().GetAsset(mc.material.albedoPath);
                }
                else if (tag == "Tex_Normal:"){
                    mc.material.normalPath = matLine.substr(12);
                    AssetManager::Get().GetAsset(mc.material.normalPath);
                }
                else if (tag == "Tex_Spec:"){
                    mc.material.specPath = matLine.substr(10);
                    AssetManager::Get().GetAsset(mc.material.specPath);
                }
                
                if (matLine.find("RigidBody: ") == 0 ||
                        matLine.find("Light: ") == 0 ||
                        matLine.find("Component: ") == 0 ||
                        matLine.find("[EndEntity]") == 0) break;
            }

            if(!mc.meshPath.empty()) AssetManager::Get().GetAsset(mc.meshPath);
            mPendingMeshEntities.emplace_back(currentEntity);
            
            m_Coordinator.AddComponent<MeshComponent>(currentEntity, mc);
        }
        
    }
    file.close();
    SyncLoadedAssets();
}

void Scene::SyncLoadedAssets() {
    if (mPendingMeshEntities.empty()) return;
    
    for (auto it = mPendingMeshEntities.begin(); it != mPendingMeshEntities.end(); )
    {
        Entity e = *it;
        auto* mc = m_Coordinator.GetComponent<MeshComponent>(e);
        
        if(mc->meshPath.empty()){
            it = mPendingMeshEntities.erase(it);
            continue;
        }
        AssetHandle handle = AssetManager::Get().GetAsset(mc->meshPath);
        bool matTexLoaded = SyncMaterial(mc->material);
        if (handle.IsReady) {
            mc->meshID = handle.iD;
            if(matTexLoaded) it = mPendingMeshEntities.erase(it);
        } else {
            ++it;
        }
    }
}

bool Scene::SyncMaterial(Material& material)
{
    bool albedoReady = material.albedoPath.empty() || AssetManager::Get().GetAsset(material.albedoPath).IsReady;
    bool normalReady = material.normalPath.empty() || AssetManager::Get().GetAsset(material.normalPath).IsReady;
    bool specReady   = material.specPath.empty()   || AssetManager::Get().GetAsset(material.specPath).IsReady;

    if (albedoReady && !material.albedoPath.empty())
        material.albedoID = AssetManager::Get().GetAsset(material.albedoPath).iD;
    
    if (normalReady && !material.normalPath.empty())
        material.normalID = AssetManager::Get().GetAsset(material.normalPath).iD;

    if (specReady && !material.specPath.empty())
        material.specID = AssetManager::Get().GetAsset(material.specPath).iD;

    return albedoReady && normalReady && specReady;
}


void Scene::RemoveEntity(Entity e)
{
    m_Coordinator.DestroyEntity(e);
}

void Scene::RenameEntity(Entity e, const char* newName)
{
    if(NameComponent* nameComponent = m_Coordinator.GetComponent<NameComponent>(e))
    {
        int count = NameExistCount(e, newName);
        if(count==0){
            nameComponent->Name = newName;
            return;
        }
        std::string name(newName);
        while(true){
            std::string suffix = "_"+std::to_string(count);
            std::string tempname = name + suffix;
            int n = NameExistCount(e, tempname.c_str());
            if(n == 0){
                nameComponent->Name = tempname;
                return;
            }
            else count++;
        }
    }
}

int Scene::NameExistCount(Entity e, const char* aName){
    int count = 0;
    const std::vector<Entity>& aliveEntities = m_Coordinator.GetAliveEntities();
    for(auto entity : aliveEntities){
        if(entity == e) continue;
        if(NameComponent* nameComponent = m_Coordinator.GetComponent<NameComponent>(entity)){
            if(nameComponent->Name == aName) count++;
        }
    }
    return count;
}
