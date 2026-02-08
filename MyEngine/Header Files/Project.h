//
//  Project.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 04/01/2026.
//

#pragma once
#include <string>
#include <filesystem>
#include <memory>
struct ProjectConfig
{
    std::string Name = "Untitled";
    std::string AssetDirectory = "Assets";
    std::string StartScene = "Scenes/MainLevel.melevel";
};

class Project
{
public:
    const ProjectConfig& GetConfig() const { return m_Config; }
    
    static std::filesystem::path GetProjectDirectory() {
        return s_ActiveProject->m_ProjectDirectory;
    }

    static std::filesystem::path GetAssetDirectory() {
        return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
    }

    static std::shared_ptr<Project> GetActiveProject() { return s_ActiveProject; }
    
    static void SetActiveScenePath(std::string& path) { s_ActiveProject->m_ActiveScenePath = path; }
    static std::string GetActiveRelativeScenePath() { return s_ActiveProject->m_ActiveScenePath; }
    static std::string GetActiveAbsoluteScenePath() { return GetAssetDirectory().string()+"/"+s_ActiveProject->m_ActiveScenePath; }


    static std::shared_ptr<Project> New(const std::filesystem::path& baseDir, const std::string& projectName);
    static std::shared_ptr<Project> Load(const std::filesystem::path& path);
    static bool SaveActive(const std::filesystem::path& path);

private:
    ProjectConfig m_Config;
    std::filesystem::path m_ProjectDirectory;
    std::string m_ActiveScenePath;
    inline static std::shared_ptr<Project> s_ActiveProject;
};
